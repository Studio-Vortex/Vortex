#include "vxpch.h"
#include "ScriptEngine.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/Buffer.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Scripting/ScriptClass.h"
#include "Vortex/Scripting/ScriptRegistry.h"
#include "Vortex/Scripting/ScriptInstance.h"
#include "Vortex/Scripting/ScriptFieldInstance.h"
#include "Vortex/Scripting/RuntimeMethodArgument.h"

#include "Vortex/Physics/3D/Physics.h"

#include "Vortex/Utils/FileSystem.h"

#include <mono/jit/jit.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>

#include <Filewatch.hpp>

namespace Vortex {

	static constexpr const char* APP_ASSEMBLY_RELOAD_SOUND_PATH = "Resources/Sounds/Compile.wav";

	struct ScriptEngineInternalData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;
			
		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path AppAssemblyFilepath;

		SharedReference<ScriptClass> ActorClass = nullptr;

		UniqueRef<filewatch::FileWatch<std::string>> AppAssemblyFilewatcher = nullptr;
		bool AssemblyReloadPending = false;
		bool DebuggingEnabled = false;
		bool MonoInitialized = false;

		SharedReference<AudioSource> AppAssemblyReloadSound = nullptr;

		std::unordered_map<std::string, SharedReference<ScriptClass>> ActorClasses;
		std::unordered_map<UUID, SharedReference<ScriptInstance>> ActorInstances;
		std::unordered_map<UUID, ScriptFieldMap> ActorScriptFields;

		// Runtime
		Scene* ContextScene = nullptr;

		// Other
		SubModule Module;
	};

	static ScriptEngineInternalData* s_Data = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		const bool assemblyModified = changeType == filewatch::Event::modified;
		const bool assemblyRemoved = changeType == filewatch::Event::removed;
		const bool reloadNotPending = !s_Data->AssemblyReloadPending;
		const bool reloadPending = s_Data->AssemblyReloadPending;

		if (assemblyModified && reloadNotPending)
		{
			s_Data->AssemblyReloadPending = true;

			// Add reload to main thread queue
			Application::Get().SubmitToMainThreadQueue([]()
			{
				s_Data->AppAssemblyFilewatcher.reset();

				ScriptEngine::ReloadAssembly();
			});
		}
	}

	void ScriptEngine::Init()
	{
		VX_PROFILE_FUNCTION();

		s_Data = new ScriptEngineInternalData();

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();
		s_Data->DebuggingEnabled = projectProps.ScriptingProps.EnableMonoDebugging;

		InitMono();
		ScriptRegistry::RegisterInternalCalls();

		const Fs::Path coreAssemblyPath = "Resources/Scripts/Vortex-ScriptCore.dll";
		bool assemblyLoaded = LoadAssembly(coreAssemblyPath);

		if (!assemblyLoaded)
		{
			VX_CONSOLE_LOG_ERROR("[Scripting] Failed to load Vortex-ScriptCore '{}'", coreAssemblyPath);
			return;
		}

		const Fs::Path appAssemblyPath = Project::GetAssetDirectory() / projectProps.ScriptingProps.ScriptBinaryPath;
		assemblyLoaded = LoadAppAssembly(appAssemblyPath);
		
		if (!assemblyLoaded)
		{
			VX_CONSOLE_LOG_ERROR("Failed to load App Assembly '{}'", appAssemblyPath);
			VX_CONSOLE_LOG_ERROR("Try re-building the project solution");
			return;
		}

		ScriptRegistry::RegisterComponents();

		LoadAssemblyClasses();
		
		ScriptRegistry::LoadPlayerPrefs();

		// Define the Actor class in the Mono runtime
		s_Data->ActorClass = SharedReference<ScriptClass>::Create("Vortex", "Actor", true);

		// Create the app assembly reload sound, this plays when the app assembly dll was changed,
		// this idea is from Epic Games
		s_Data->AppAssemblyReloadSound = AudioSource::Create(APP_ASSEMBLY_RELOAD_SOUND_PATH);
		PlaybackDevice audioDevice = s_Data->AppAssemblyReloadSound->GetPlaybackDevice();
		audioDevice.GetSound().SetSpacialized(false); // we don't need spacialization

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Script-Engine";
		moduleProps.APIVersion = Version(1, 2, 0);
		moduleProps.RequiredModules = {};
		s_Data->Module.Init(moduleProps);

		Application::Get().AddModule(s_Data->Module);
	}

	void ScriptEngine::Shutdown()
	{
		VX_PROFILE_FUNCTION();

		ShutdownMono();

		ScriptRegistry::SavePlayerPrefs();

		// NOTE:
		// We need to manually clean up this audio source because
		// it is not attached to an actor in a scene, if it was,
		// the scene would be responsible for cleaning it up
		// as mentioned in AudioSource::~AudioSource()
		s_Data->AppAssemblyReloadSound->GetPlaybackDevice().Shutdown(Audio::GetContext());

		Application::Get().RemoveModule(s_Data->Module);
		s_Data->Module.Shutdown();

		delete s_Data;
		s_Data = nullptr;
	}

	void ScriptEngine::InitMono()
	{
		VX_PROFILE_FUNCTION();

		if (s_Data->MonoInitialized)
			return;

		mono_set_assemblies_path("mono/lib");

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		if (s_Data->DebuggingEnabled)
		{
			uint32_t debugListenerPort = projectProps.ScriptingProps.DebugListenerPort;

			const char* argv[2] = {
				fmt::format("--debugger-agent=transport=dt_socket,address=127.0.0.1:{0},server=y,suspend=n,loglevel=3,logfile=Resources/Logs/MonoDebugger.txt", debugListenerPort).c_str(),
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		s_Data->RootDomain = mono_jit_init("VortexJITRuntime");
		VX_CORE_ASSERT(s_Data->RootDomain, "Failed to initialize mono!");

		if (s_Data->DebuggingEnabled)
		{
			mono_debug_domain_create(s_Data->RootDomain);
		}

		mono_thread_set_main(mono_thread_current());

		s_Data->MonoInitialized = true;
	}

	void ScriptEngine::ShutdownMono()
	{
		VX_PROFILE_FUNCTION();

		if (!s_Data->MonoInitialized)
		{
			VX_CORE_ASSERT(false, "Trying to shutdown mono multiple times");
			return;
		}

		s_Data->AppDomain = nullptr;
		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;

		s_Data->MonoInitialized = false;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		VX_PROFILE_FUNCTION();

		char name[20] = "VortexScriptRuntime";
		s_Data->AppDomain = mono_domain_create_appdomain(name, nullptr);
		mono_domain_set(s_Data->AppDomain, true);
		mono_domain_set_config(s_Data->AppDomain, ".", "");

		s_Data->CoreAssemblyFilepath = filepath;
		s_Data->CoreAssembly = ScriptUtils::LoadMonoAssembly(filepath, s_Data->DebuggingEnabled);

		if (s_Data->CoreAssembly == nullptr)
			return false;

		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);

		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		VX_PROFILE_FUNCTION();

		s_Data->AppAssemblyFilepath = filepath;
		s_Data->AppAssembly = ScriptUtils::LoadMonoAssembly(filepath, s_Data->DebuggingEnabled);

		if (s_Data->AppAssembly == nullptr)
			return false;

		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);

		const ProjectProperties& projectProps = Project::GetActive()->GetProperties();
		s_Data->AppAssemblyFilewatcher = CreateUnique<filewatch::FileWatch<std::string>>((Project::GetAssetDirectory() / projectProps.ScriptingProps.ScriptBinaryPath).string(), OnAppAssemblyFileSystemEvent);
		s_Data->AssemblyReloadPending = false;

		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		VX_PROFILE_FUNCTION();

		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);

		ScriptRegistry::RegisterInternalCalls();

		bool assemblyLoaded = LoadAssembly(s_Data->CoreAssemblyFilepath);

		if (!assemblyLoaded)
		{
			VX_CONSOLE_LOG_ERROR("Failed to load Vortex-ScriptCore '{}'", s_Data->CoreAssemblyFilepath);
			return;
		}

		assemblyLoaded = LoadAppAssembly(s_Data->AppAssemblyFilepath);

		if (!assemblyLoaded)
		{
			VX_CONSOLE_LOG_ERROR("Failed to load App Assembly '{}'", s_Data->AppAssemblyFilepath);
			return;
		}

		ScriptRegistry::RegisterComponents();

		LoadAssemblyClasses();

		s_Data->ActorClass = SharedReference<ScriptClass>::Create("Vortex", "Actor", true);

		// play the assembly reload sound only in the editor
		// can we come up with a better way of checking for runtime?
		// perhaps a macro VX_RUNTIME could be defined in VortexRuntimeApp.cpp
		// that way we can actually strip code from the runtime app
		if (!Application::Get().IsRuntime())
		{
			s_Data->AppAssemblyReloadSound->GetPlaybackDevice().Play();
		}
	}

	void ScriptEngine::OnRuntimeStart(Scene* contextScene)
	{
		s_Data->ContextScene = contextScene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->ContextScene = nullptr;
		s_Data->ActorInstances.clear();
	}

	bool ScriptEngine::ActorClassExists(const std::string& fullyQualifiedClassName)
	{
		return s_Data->ActorClasses.contains(fullyQualifiedClassName);
	}

	bool ScriptEngine::HasValidScriptClass(Actor actor)
	{
		if (!actor.HasComponent<ScriptComponent>())
		{
			return false;
		}

		const ScriptComponent& scriptComponent = actor.GetComponent<ScriptComponent>();
		const std::string& className = scriptComponent.ClassName;

		if (className.empty())
		{
			return false;
		}

		return ActorClassExists(className);
	}

	bool ScriptEngine::ActorInstanceExists(UUID actorUUID)
	{
		return s_Data->ActorInstances.contains(actorUUID);
	}

	bool ScriptEngine::ScriptInstanceHasMethod(Actor actor, ManagedMethod method)
	{
		if (!actor.HasComponent<ScriptComponent>())
		{
			VX_CORE_ASSERT(false, "this should never happen!");
			return false;
		}

		UUID actorUUID = actor.GetUUID();
		if (!ActorInstanceExists(actorUUID))
		{
			return false;
		}

		SharedReference<ScriptInstance> instance = GetActorScriptInstance(actorUUID);
		if (instance == nullptr)
		{
			return false;
		}

		return instance->MethodExists(method);
	}

	void ScriptEngine::RT_ActorConstructor(UUID actorUUID, MonoObject* instance)
	{
		VX_PROFILE_FUNCTION();

		Scene* contextScene = GetContextScene();
		VX_CORE_ASSERT(contextScene, "Invalid scene");

		MonoMethod* actorConstructor = s_Data->ActorClass->GetMethod(".ctor", 1);

		void* param = (void*)&actorUUID;
		ScriptUtils::InvokeMethod(instance, actorConstructor, &param);
	}

	void ScriptEngine::RT_CreateActorScriptInstance(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		Scene* contextScene = GetContextScene();
		VX_CORE_ASSERT(contextScene, "Invalid scene");

		const UUID actorUUID = actor.GetUUID();

		const ScriptComponent& scriptComponent = actor.GetComponent<ScriptComponent>();
		const std::string& className = scriptComponent.ClassName;

		if (className.empty())
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to create script instance for Actor '{}' without a class name!", actor.GetName());
			return;
		}

		VX_CORE_ASSERT(!ActorInstanceExists(actorUUID), "Instance was already found with UUID!");
		VX_CORE_ASSERT(ActorClassExists(className), "Actor Class was not found in Actor Classes Map!");

		SharedReference<ScriptClass> scriptClass = GetActorClass(className);
		SharedReference<ScriptInstance> instance = SharedReference<ScriptInstance>::Create(scriptClass);
		
		// Invoke C# Actor class constructor
		instance->InvokeConstructor(actorUUID);

		s_Data->ActorInstances[actorUUID] = instance;

		// Copy field values
		auto it = s_Data->ActorScriptFields.find(actorUUID);
		if (it == s_Data->ActorScriptFields.end())
		{
			return;
		}

		const ScriptFieldMap& fields = it->second;

		for (const auto& [name, fieldInstance] : fields)
		{
			instance->SetFieldValueInternal(name, fieldInstance.GetDataBuffer());
		}
	}

	void ScriptEngine::RT_InstantiateActor(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		Scene* contextScene = GetContextScene();
		VX_CORE_ASSERT(contextScene, "Invalid scene");

		if (!actor)
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to instantiate invalid Actor!");
			return;
		}

		if (!actor.HasComponent<ScriptComponent>())
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to instantiate Actor '{}' without script component!", actor.GetName());
			return;
		}

		if (!contextScene->IsRunning())
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to instantiate Actor '{}' while scene is stopped!", actor.GetName());
			return;
		}

		if (!HasValidScriptClass(actor))
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to instantiate Actor '{}' with no script class!", actor.GetName());
			return;
		}

		// Create the script instance
		RT_CreateActorScriptInstance(actor);
		VX_CORE_ASSERT(ActorInstanceExists(actor), "Actor script instance not instantiated properly!");

		// Invoke Actor.OnAwake, Actor.OnCreate
		ManagedMethod methods[] = { ManagedMethod::OnAwake, ManagedMethod::OnCreate };
		const size_t methodCount = VX_ARRAYSIZE(methods);

		for (size_t i = 0; i < methodCount; i++)
		{
			actor.CallMethod(methods[i]);
		}
	}

	bool ScriptEngine::Invoke(const std::string& methodName, Actor actor, const std::vector<RuntimeMethodArgument>& argumentList)
	{
		VX_PROFILE_FUNCTION();

		if (methodName.empty())
		{
			VX_CORE_ASSERT(false, "Trying to call non-existent method!");
			return false;
		}

		ManagedMethod method = Utils::ManagedMethodFromString(methodName);

		return Invoke(method, actor, argumentList);
	}

	bool ScriptEngine::Invoke(ManagedMethod method, Actor actor, const std::vector<RuntimeMethodArgument>& argumentList)
	{
		VX_PROFILE_FUNCTION();

		if (!actor)
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.{} on invalid actor!", Utils::StringFromManagedMethod(method));
			return false;
		}

		const UUID actorUUID = actor.GetUUID();
		const ScriptComponent& scriptComponent = actor.GetComponent<ScriptComponent>();

		VX_CORE_ASSERT(ActorClassExists(scriptComponent.ClassName), "Class was not found in Actor Class Map!");
		VX_CORE_ASSERT(ActorInstanceExists(actorUUID), "Actor was not instantiated properly!");

		if (!ActorInstanceExists(actorUUID))
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Failed to find ScriptInstance for Actor with Tag: {}", actor.GetName());
			return false;
		}

		SharedReference<ScriptInstance> instance = GetActorScriptInstance(actorUUID);
		VX_CORE_ASSERT(instance, "Invalid script instance!");

		if (instance == nullptr)
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Actor.{} on actor '{}' with invalid script instance!", Utils::StringFromManagedMethod(method), actor.GetName());
			return false;
		}

		switch (method)
		{
			case ManagedMethod::OnAwake:
			{
				instance->InvokeOnAwake();
				break;
			}
			case ManagedMethod::OnCreate:
			{
				instance->InvokeOnCreate();
				break;
			}
			case ManagedMethod::OnUpdateDelta: // fallthrough
			case ManagedMethod::OnUpdate:
			{
				VX_CORE_ASSERT(argumentList.size() >= 1, "Expected arguments to managed method!");
				if (argumentList.size() < 1)
				{
					return false;
				}

				const RuntimeMethodArgument& arg0 = argumentList.front();
				VX_CORE_ASSERT(arg0.Is(RuntimeArgumentType::TimeStep), "unexpected argument type!");
				instance->InvokeOnUpdate(arg0.AsTimeStep());
				break;
			}
			case ManagedMethod::OnDestroy:
			{
				instance->InvokeOnDestroy();
				
				// Remove the instance from the script instance map
				s_Data->ActorInstances.erase(actorUUID);

				break;
			}
			case ManagedMethod::OnCollisionEnter:
			{
				VX_CORE_ASSERT(argumentList.size() >= 1, "Expected arguments to managed method!");
				if (argumentList.size() < 1)
				{
					return false;
				}

				const RuntimeMethodArgument& arg0 = argumentList.front();
				VX_CORE_ASSERT(arg0.Is(RuntimeArgumentType::Collision), "unexpected argument type!");
				instance->InvokeOnCollisionEnter(arg0.AsCollision());
				break;
			}
			case ManagedMethod::OnCollisionExit:
			{
				VX_CORE_ASSERT(argumentList.size() >= 1, "Expected arguments to managed method!");
				if (argumentList.size() < 1)
				{
					return false;
				}

				const RuntimeMethodArgument& arg0 = argumentList.front();
				VX_CORE_ASSERT(arg0.Is(RuntimeArgumentType::Collision), "unexpected argument type!");
				instance->InvokeOnCollisionExit(arg0.AsCollision());
				break;
			}
			case ManagedMethod::OnTriggerEnter:
			{
				VX_CORE_ASSERT(argumentList.size() >= 1, "Expected arguments to managed method!");
				if (argumentList.size() < 1)
				{
					return false;
				}

				const RuntimeMethodArgument& arg0 = argumentList.front();
				VX_CORE_ASSERT(arg0.Is(RuntimeArgumentType::Collision), "unexpected argument type!");
				instance->InvokeOnTriggerEnter(arg0.AsCollision());
				break;
			}
			case ManagedMethod::OnTriggerExit:
			{
				VX_CORE_ASSERT(argumentList.size() >= 1, "Expected arguments to managed method!");
				if (argumentList.size() < 1)
				{
					return false;
				}

				const RuntimeMethodArgument& arg0 = argumentList.front();
				VX_CORE_ASSERT(arg0.Is(RuntimeArgumentType::Collision), "unexpected argument type!");
				instance->InvokeOnTriggerExit(arg0.AsCollision());
				break;
			}
			case ManagedMethod::OnFixedJointDisconnected:
			{
				VX_CORE_ASSERT(argumentList.size() >= 1, "Expected arguments to managed method!");
				if (argumentList.size() < 1)
				{
					return false;
				}

				const RuntimeMethodArgument& arg0 = argumentList.front();
				VX_CORE_ASSERT(arg0.Is(RuntimeArgumentType::ForceAndTorque), "unexpected argument type!");
				instance->InvokeOnFixedJointDisconnected(arg0.AsForceAndTorque());
				break;
			}
			case ManagedMethod::OnEnable:
			{
				instance->InvokeOnEnable();
				break;
			}
			case ManagedMethod::OnDisable:
			{
				instance->InvokeOnDisable();
				break;
			}
			case ManagedMethod::OnDebugRender:
			{
				instance->InvokeOnDebugRender();
				break;
			}
			case ManagedMethod::OnGuiRender:
			{
				instance->InvokeOnGuiRender();
				break;
			}
		}

		return true;
	}

	SharedReference<ScriptClass> ScriptEngine::GetCoreActorClass()
	{
		return s_Data->ActorClass;
	}

	SharedReference<ScriptInstance> ScriptEngine::GetActorScriptInstance(UUID uuid)
	{
		VX_PROFILE_FUNCTION();

		if (ActorInstanceExists(uuid))
		{
			return s_Data->ActorInstances[uuid];
		}

		return nullptr;
	}

	SharedReference<ScriptClass> ScriptEngine::GetActorClass(const std::string& name)
	{
		VX_PROFILE_FUNCTION();

		if (ActorClassExists(name))
		{
			return s_Data->ActorClasses[name];
		}

		return nullptr;
	}

	std::unordered_map<std::string, SharedReference<ScriptClass>> ScriptEngine::GetClasses()
	{
		return s_Data->ActorClasses;
	}

	const ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(actor, "Actor was invalid!");
		VX_CORE_ASSERT(s_Data->ActorScriptFields.contains(actor.GetUUID()), "Actor was not found in script field map!");

		return s_Data->ActorScriptFields[actor.GetUUID()];
	}

	ScriptFieldMap& ScriptEngine::GetMutableScriptFieldMap(Actor actor)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(actor, "Actor was invalid!");

		return s_Data->ActorScriptFields[actor.GetUUID()];
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		VX_PROFILE_FUNCTION();

		SharedReference<ScriptInstance> instance = GetActorScriptInstance(uuid);
		return instance->GetManagedObject();
	}

	std::vector<MonoAssemblyTypeInfo> ScriptEngine::GetCoreAssemblyTypeInfo()
	{
		return ScriptUtils::GetAssemblyTypeInfo(s_Data->CoreAssembly);
	}

	size_t ScriptEngine::GetScriptInstanceCount()
	{
		return s_Data->ActorInstances.size();
	}

	Scene* ScriptEngine::GetContextScene()
	{
		return s_Data->ContextScene;
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	MonoDomain* ScriptEngine::GetAppDomain()
	{
		return s_Data->AppDomain;
	}

	MonoImage* ScriptEngine::GetAppAssemblyImage()
	{
		return s_Data->AppAssemblyImage;
	}

	void ScriptEngine::LoadAssemblyClasses(bool displayClasses)
	{
		VX_PROFILE_FUNCTION();

		s_Data->ActorClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* actorClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Vortex", "Actor");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			
			std::string fullName = className;
			if (strlen(nameSpace) != 0)
			{
				fullName = fmt::format("{}.{}", nameSpace, className);
			}

			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, className);

			if (!monoClass || monoClass == actorClass)
				continue;

			const bool isActorClass = mono_class_is_subclass_of(monoClass, actorClass, false);

			if (!isActorClass)
				continue;

			SharedReference<ScriptClass> scriptClass = SharedReference<ScriptClass>::Create(nameSpace, className);
			s_Data->ActorClasses[fullName] = scriptClass;

			const int fieldCount = mono_class_num_fields(monoClass);

			if (displayClasses)
			{
				VX_CONSOLE_LOG_INFO("{} has {} fields: ", className, fieldCount);
			}

			void* iterator = nullptr;

			while (MonoClassField* classField = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(classField);
				uint32_t flags = mono_field_get_flags(classField);

				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(classField);
					ScriptFieldType fieldType = ScriptUtils::MonoTypeToScriptFieldType(type);

					if (displayClasses)
					{
						VX_CONSOLE_LOG_INFO("  {} ({})", fieldName, ScriptUtils::ScriptFieldTypeToString(fieldType));
					}

					ScriptField scriptField = { fieldType, fieldName, classField };
					scriptClass->SetField(fieldName, scriptField);
				}
			}
		}
	}

}

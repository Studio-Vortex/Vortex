#include "vxpch.h"
#include "ScriptEngine.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/Buffer.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Scene/Entity.h"
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

		SharedReference<ScriptClass> EntityClass = nullptr;

		UniqueRef<filewatch::FileWatch<std::string>> AppAssemblyFilewatcher = nullptr;
		bool AssemblyReloadPending = false;
		bool DebuggingEnabled = false;
		bool MonoInitialized = false;

		SharedReference<AudioSource> AppAssemblyReloadSound = nullptr;

		std::unordered_map<std::string, SharedReference<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, SharedReference<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// Runtime
		Scene* ContextScene = nullptr;

		// Other
		SubModule Module;
	};

	static ScriptEngineInternalData* s_Data = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		const bool assemblyModified = changeType == filewatch::Event::modified;
		const bool reloadNotPending = !s_Data->AssemblyReloadPending;

		if (assemblyModified && reloadNotPending)
		{
			s_Data->AssemblyReloadPending = true;

			// Add reload to main thread queue
			Application::Get().SubmitToMainThreadQueue([]()
			{
				s_Data->AppAssemblyFilewatcher.reset();

				ScriptEngine::ReloadAssembly();
				s_Data->AppAssemblyReloadSound->GetPlaybackDevice().Play();
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
		ScriptRegistry::RegisterMethods();

		std::filesystem::path coreAssemblyPath = "Resources/Scripts/Vortex-ScriptCore.dll";
		bool assemblyLoaded = LoadAssembly(coreAssemblyPath);

		if (!assemblyLoaded)
		{
			VX_CONSOLE_LOG_ERROR("Failed to load Vortex-ScriptCore from path: {}", coreAssemblyPath);
			return;
		}

		std::filesystem::path appAssemblyPath = Project::GetAssetDirectory() / projectProps.ScriptingProps.ScriptBinaryPath;
		assemblyLoaded = LoadAppAssembly(appAssemblyPath);
		
		if (!assemblyLoaded)
		{
			VX_CONSOLE_LOG_ERROR("Failed to load App Assembly from path: {}", appAssemblyPath);
			return;
		}

		LoadAssemblyClasses();

		ScriptRegistry::RegisterComponents();

		s_Data->EntityClass = SharedReference<ScriptClass>::Create("Vortex", "Entity", true);
		s_Data->AppAssemblyReloadSound = AudioSource::Create(APP_ASSEMBLY_RELOAD_SOUND_PATH);
		s_Data->AppAssemblyReloadSound->GetPlaybackDevice().GetSound().SetSpacialized(false);

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

		// NOTE:
		// We need to manually clean up this audio source because
		// it is not attached to an entity in a scene, if it was,
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

		LoadAssembly(s_Data->CoreAssemblyFilepath);
		LoadAppAssembly(s_Data->AppAssemblyFilepath);

		LoadAssemblyClasses();

		ScriptRegistry::RegisterComponents();

		s_Data->EntityClass = SharedReference<ScriptClass>::Create("Vortex", "Entity", true);
	}

	void ScriptEngine::OnRuntimeStart(Scene* contextScene)
	{
		s_Data->ContextScene = contextScene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->ContextScene = nullptr;
		s_Data->EntityInstances.clear();
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullyQualifiedClassName)
	{
		return s_Data->EntityClasses.contains(fullyQualifiedClassName);
	}

	bool ScriptEngine::EntityInstanceExists(UUID entityUUID)
	{
		return s_Data->EntityInstances.contains(entityUUID);
	}

	void ScriptEngine::EntityConstructorRuntime(UUID entityUUID, MonoObject* instance)
	{
		MonoMethod* entityConstructor = s_Data->EntityClass->GetMethod(".ctor", 1);

		void* param = (void*)&entityUUID;
		ScriptUtils::InvokeMethod(instance, entityConstructor, &param);
	}

	void ScriptEngine::RT_CreateEntityScriptInstance(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		const UUID entityUUID = entity.GetUUID();

		const ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();

		VX_CORE_ASSERT(!EntityInstanceExists(entityUUID), "Instance was already found with UUID!");
		VX_CORE_ASSERT(EntityClassExists(scriptComponent.ClassName), "Entity Class was not found in Entity Classes Map!");

		SharedReference<ScriptClass> scriptClass = GetEntityClass(scriptComponent.ClassName);
		SharedReference<ScriptInstance> instance = SharedReference<ScriptInstance>::Create(scriptClass, entity);
		
		// Invoke C# Entity class constructor
		instance->InvokeConstructor(entity);

		s_Data->EntityInstances[entityUUID] = instance;

		// Copy field values
		auto it = s_Data->EntityScriptFields.find(entityUUID);

		if (it == s_Data->EntityScriptFields.end())
		{
			return;
		}

		const ScriptFieldMap& fields = it->second;

		for (const auto& [name, fieldInstance] : fields)
		{
			instance->SetFieldValueInternal(name, fieldInstance.GetDataBuffer());
		}
	}

	bool ScriptEngine::Invoke(const std::string& methodName, Entity entity, const std::vector<RuntimeMethodArgument>& argumentList)
	{
		if (methodName.empty())
		{
			VX_CORE_ASSERT(false, "Trying to call non-existent method!");
			return false;
		}

		ManagedMethod method = Utils::ManagedMethodFromString(methodName);

		return Invoke(method, entity, argumentList);
	}

	bool ScriptEngine::Invoke(ManagedMethod method, Entity entity, const std::vector<RuntimeMethodArgument>& argumentList)
	{
		VX_PROFILE_FUNCTION();

		if (!entity)
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Entity.{} on invalid entity!");
			return false;
		}

		const UUID entityUUID = entity.GetUUID();
		const ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();

		VX_CORE_ASSERT(EntityClassExists(scriptComponent.ClassName), "Class was not found in Entity Class Map!");
		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		if (!EntityInstanceExists(entityUUID))
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Failed to find ScriptInstance for Entity with Tag: {}", entity.GetName());
			return false;
		}

		SharedReference<ScriptInstance> instance = GetEntityScriptInstance(entityUUID);
		VX_CORE_ASSERT(instance, "Invalid script instance!");

		if (instance == nullptr)
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Calling Entity.{} on entity '{}' with invalid script instance!", Utils::StringFromManagedMethod(method), entity.GetName());
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
				instance->InvokeOnUpdate(arg0.Delta);
				break;
			}
			case ManagedMethod::OnDestroy:
			{
				instance->InvokeOnDestroy();
				
				// Remove the instance from the script instance map
				s_Data->EntityInstances.erase(entityUUID);

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
				instance->InvokeOnCollisionEnter(arg0.CollisionArg);
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
				instance->InvokeOnCollisionExit(arg0.CollisionArg);
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
				instance->InvokeOnTriggerEnter(arg0.CollisionArg);
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
				instance->InvokeOnTriggerExit(arg0.CollisionArg);
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
				instance->InvokeOnFixedJointDisconnected(arg0.ForceAndTorque);
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
			case ManagedMethod::OnGui:
			{
				instance->InvokeOnGui();
				break;
			}
		}

		return true;
	}

	SharedReference<ScriptClass> ScriptEngine::GetCoreEntityClass()
	{
		return s_Data->EntityClass;
	}

	SharedReference<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID uuid)
	{
		if (EntityInstanceExists(uuid))
		{
			return s_Data->EntityInstances[uuid];
		}

		return nullptr;
	}

	SharedReference<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (EntityClassExists(name))
		{
			return s_Data->EntityClasses[name];
		}

		return nullptr;
	}

	std::unordered_map<std::string, SharedReference<ScriptClass>> ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}

	const ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		VX_CORE_ASSERT(entity, "Entity was invalid!");
		VX_CORE_ASSERT(s_Data->EntityScriptFields.contains(entity.GetUUID()), "Entity was not found in script field map!");

		return s_Data->EntityScriptFields[entity.GetUUID()];
	}

	ScriptFieldMap& ScriptEngine::GetMutableScriptFieldMap(Entity entity)
	{
		VX_CORE_ASSERT(entity, "Entity was invalid!");

		return s_Data->EntityScriptFields[entity.GetUUID()];
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		SharedReference<ScriptInstance> instance = GetEntityScriptInstance(uuid);
		return instance->GetManagedObject();
	}

	std::vector<MonoAssemblyTypeInfo> ScriptEngine::GetCoreAssemblyTypeInfo()
	{
		return ScriptUtils::GetAssemblyTypeInfo(s_Data->CoreAssembly);
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

	void ScriptEngine::RuntimeInstantiateEntity(Entity entity)
	{
		if (!entity)
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to instantiate invalid Entity!");
			return;
		}

		if (!entity.HasComponent<ScriptComponent>())
		{
			VX_CONSOLE_LOG_ERROR("[Script Engine] Trying to instantiate Entity '{}' without script component!", entity.GetName());
			return;
		}

		Scene* contextScene = GetContextScene();
		VX_CORE_ASSERT(contextScene, "Invalid scene");

		if (!contextScene->IsRunning())
		{
			return;
		}

		// Create the instance
		ScriptEngine::RT_CreateEntityScriptInstance(entity);
		VX_CORE_ASSERT(EntityInstanceExists(entity), "Entity script instance not instantiated properly!");

		// Invoke Entity.OnAwake
		ScriptEngine::Invoke(ManagedMethod::OnAwake, entity);
		// Invoke Entity.OnCreate
		ScriptEngine::Invoke(ManagedMethod::OnCreate, entity);
	}

	void ScriptEngine::LoadAssemblyClasses(bool displayClasses)
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Vortex", "Entity");

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

			if (!monoClass || monoClass == entityClass)
				continue;

			const bool isEntityClass = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntityClass)
				continue;

			SharedReference<ScriptClass> scriptClass = SharedReference<ScriptClass>::Create(nameSpace, className);
			s_Data->EntityClasses[fullName] = scriptClass;

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

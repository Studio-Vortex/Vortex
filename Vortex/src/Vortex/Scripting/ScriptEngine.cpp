#include "vxpch.h"
#include "ScriptEngine.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Core/Application.h"
#include "Vortex/Core/Buffer.h"
#include "Vortex/Scripting/ScriptRegistry.h"
#include "Vortex/Scripting/ScriptInstance.h"
#include "Vortex/Scripting/ScriptFieldInstance.h"
#include "Vortex/Scripting/ScriptClass.h"
#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Utils/FileSystem.h"

#include <mono/jit/jit.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>

#include <Filewatch.hpp>

namespace Vortex {

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

		SharedReference<AudioSource> AppAssemblyReloadSound = nullptr;

		std::unordered_map<std::string, SharedReference<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, SharedReference<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// Runtime
		Scene* ContextScene = nullptr;
	};

	static ScriptEngineInternalData* s_Data = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (!s_Data->AssemblyReloadPending && changeType == filewatch::Event::modified)
		{
			s_Data->AssemblyReloadPending = true;

			// Add reload to main thread queue
			Application::Get().SubmitToMainThreadQueue([]()
			{
				s_Data->AppAssemblyFilewatcher.reset();

				ScriptEngine::ReloadAssembly();
				s_Data->AppAssemblyReloadSound->Play();
			});
		}
	}

	void ScriptEngine::Init()
	{
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
		s_Data->AppAssemblyReloadSound = AudioSource::Create("Resources/Sounds/Compile.wav");
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete s_Data;
		s_Data = nullptr;
	}

	void ScriptEngine::InitMono()
	{
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

		MonoDomain* rootDomain = mono_jit_init("VortexJITRuntime");
		VX_CORE_ASSERT(rootDomain, "Root Domain was null pointer!");

		// Store the root domain pointer
		s_Data->RootDomain = rootDomain;

		if (s_Data->DebuggingEnabled)
		{
			mono_debug_domain_create(s_Data->RootDomain);
		}

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
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

	void ScriptEngine::ConstructEntityRuntime(UUID entityUUID, MonoObject* instance)
	{
		MonoMethod* constructor = s_Data->EntityClass->GetMethod(".ctor", 1);

		void* param = &entityUUID;
		ScriptUtils::InvokeMethod(instance, constructor, &param);
	}

	void ScriptEngine::CreateEntityScriptInstanceRuntime(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		const ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();

		VX_CORE_ASSERT(!EntityInstanceExists(entityUUID), "Instance was already found with UUID!");
		VX_CORE_ASSERT(EntityClassExists(scriptComponent.ClassName), "Entity Class was not found in Entity Classes Map!");

		SharedReference<ScriptClass> scriptClass = GetEntityClass(scriptComponent.ClassName);
		SharedReference<ScriptInstance> instance = SharedReference<ScriptInstance>::Create(scriptClass, entity);
		s_Data->EntityInstances[entityUUID] = instance;

		// Copy field values
		auto it = s_Data->EntityScriptFields.find(entityUUID);

		if (it == s_Data->EntityScriptFields.end())
			return;

		const ScriptFieldMap& fields = it->second;

		for (const auto& [name, fieldInstance] : fields)
		{
			instance->SetFieldValueInternal(name, fieldInstance.GetDataBuffer());
		}
	}

	void ScriptEngine::OnAwakeEntity(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		auto& scriptComponent = entity.GetComponent<ScriptComponent>();

		VX_CORE_ASSERT(EntityClassExists(scriptComponent.ClassName), "Class was not found in Entity Class Map!");

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnAwake();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		auto& scriptComponent = entity.GetComponent<ScriptComponent>();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnCreate();
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, TimeStep delta)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");
		
		if (EntityInstanceExists(entityUUID))
		{
			GetEntityScriptInstance(entityUUID)->InvokeOnUpdate(delta);
		}
		else
		{
			VX_CONSOLE_LOG_ERROR("Failed to find ScriptInstance for Entity with Tag: {}", entity.GetName());
		}
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		SharedReference<ScriptInstance> instance = GetEntityScriptInstance(entityUUID);
		instance->InvokeOnDestroy();

		s_Data->EntityInstances.erase(entityUUID);
	}

	void ScriptEngine::OnCollisionEnterEntity(Entity entity, Collision& collision)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnCollisionEnter(collision);
	}

	void ScriptEngine::OnCollisionExitEntity(Entity entity, Collision& collision)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnCollisionExit(collision);
	}

	void ScriptEngine::OnTriggerEnterEntity(Entity entity, Collision& collision)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnTriggerEnter(collision);
	}

	void ScriptEngine::OnTriggerExitEntity(Entity entity, Collision& collision)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnTriggerExit(collision);
	}

	void ScriptEngine::OnFixedJointDisconnected(Entity entity, const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnFixedJointDisconnected(forceAndTorque);
	}

	void ScriptEngine::OnRaycastCollisionEntity(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnRaycastCollision();
	}

	void ScriptEngine::OnEnabled(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnEnabled();
	}

	void ScriptEngine::OnDisabled(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnDisabled();
	}

	void ScriptEngine::OnGuiEntity(Entity entity)
	{
		UUID entityUUID = entity.GetUUID();

		VX_CORE_ASSERT(EntityInstanceExists(entityUUID), "Entity was not instantiated properly!");

		GetEntityScriptInstance(entityUUID)->InvokeOnGui();
	}

	SharedReference<ScriptClass> ScriptEngine::GetCoreEntityClass()
	{
		return s_Data->EntityClass;
	}

	SharedReference<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID uuid)
	{
		if (s_Data->EntityInstances.contains(uuid))
			return s_Data->EntityInstances[uuid];

		return nullptr;
	}

	SharedReference<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (s_Data->EntityClasses.contains(name))
			return s_Data->EntityClasses[name];

		return nullptr;
	}

	std::unordered_map<std::string, SharedReference<ScriptClass>> ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}

	const ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		VX_CORE_ASSERT(entity, "Entity was invalid!");

		return s_Data->EntityScriptFields[entity.GetUUID()];
	}

	ScriptFieldMap& ScriptEngine::GetMutableScriptFieldMap(Entity entity)
	{
		VX_CORE_ASSERT(entity, "Entity was invalid!");

		return s_Data->EntityScriptFields[entity.GetUUID()];
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		auto it = s_Data->EntityInstances.find(uuid);

		if (it != s_Data->EntityInstances.end())
		{
			return it->second->GetManagedObject();
		}
		
		Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(uuid);
		VX_CONSOLE_LOG_ERROR("Failed to find ScriptInstance for Entity with Tag: {}", entity.GetName());
		return nullptr;
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

	void ScriptEngine::DuplicateScriptInstance(Entity entity, Entity targetEntity)
	{
		if (!entity.HasComponent<ScriptComponent>() || !targetEntity.HasComponent<ScriptComponent>())
			return;

		const auto& srcScriptComponent = entity.GetComponent<ScriptComponent>();
		auto& dstSriptComponent = targetEntity.GetComponent<ScriptComponent>();

		if (srcScriptComponent.ClassName != dstSriptComponent.ClassName)
		{
			auto entityClasses = ScriptEngine::GetClasses();

			//TODO
		}
	}

	void ScriptEngine::LoadAssemblyClasses(bool displayClassNames)
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
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, className);

			if (!monoClass || monoClass == entityClass)
				continue;

			bool isEntityClass = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntityClass)
				continue;

			SharedReference<ScriptClass> scriptClass = SharedReference<ScriptClass>::Create(nameSpace, className);
			s_Data->EntityClasses[fullName] = scriptClass;

			int fieldCount = mono_class_num_fields(monoClass);

			if (displayClassNames)
				VX_CONSOLE_LOG_INFO("{} has {} fields: ", className, fieldCount);

			void* iterator = nullptr;

			while (MonoClassField* classField = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(classField);
				uint32_t flags = mono_field_get_flags(classField);

				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(classField);
					ScriptFieldType fieldType = ScriptUtils::MonoTypeToScriptFieldType(type);

					if (displayClassNames)
						VX_CONSOLE_LOG_INFO("  {} ({})", fieldName, ScriptUtils::ScriptFieldTypeToString(fieldType));

					ScriptField scriptField = { fieldType, fieldName, classField };
					scriptClass->SetField(fieldName, scriptField);
				}
			}
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		return ScriptUtils::InstantiateClass(monoClass);
	}

}

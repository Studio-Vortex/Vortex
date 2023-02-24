#include "vxpch.h"
#include "ScriptEngine.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Core/Application.h"
#include "Vortex/Core/Buffer.h"
#include "Vortex/Scripting/ScriptRegistry.h"
#include "Vortex/Scripting/ScriptUtils.h"
#include "Vortex/Audio/AudioSource.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Utils/FileSystem.h"

#include <mono/jit/jit.h>
#include <mono/metadata/class.h>
#include <mono/metadata/object.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-debug.h>

#include <Filewatch.hpp>

namespace Vortex {

	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{ "System.Single",  ScriptFieldType::Float   },
		{ "System.Double",  ScriptFieldType::Double  },
		{ "System.Boolean", ScriptFieldType::Bool    },
		{ "System.Char",    ScriptFieldType::Char    },
		{ "System.Int16",   ScriptFieldType::Short   },
		{ "System.Int32",   ScriptFieldType::Int     },
		{ "System.Int64",   ScriptFieldType::Long    },
		{ "System.Byte",    ScriptFieldType::Byte    },
		{ "System.UInt16",  ScriptFieldType::UShort  },
		{ "System.UInt32",  ScriptFieldType::UInt    },
		{ "System.UInt64",  ScriptFieldType::ULong   },
		{ "Vortex.Vector2", ScriptFieldType::Vector2 },
		{ "Vortex.Vector3", ScriptFieldType::Vector3 },
		{ "Vortex.Vector4", ScriptFieldType::Vector4 },
		{ "Vortex.Color3",  ScriptFieldType::Color3  },
		{ "Vortex.Color4",  ScriptFieldType::Color4  },
		{ "Vortex.Entity",  ScriptFieldType::Entity  },
	};

	namespace Utils {

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& filepath, bool loadPdb = false)
		{
			UniqueBuffer fileData = FileSystem::ReadBinary(filepath);

			if (!fileData)
			{
				return nullptr;
			}

			// NOTE: We can't use this image for anything other than loading the assembly
			//       because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), fileData.Size(), 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				VX_CONSOLE_LOG_ERROR("Mono Assembly Error: {}", errorMessage);
				return nullptr;
			}

			if (loadPdb)
			{
				std::filesystem::path assemblyPath = filepath;
				std::filesystem::path pdbPath = assemblyPath.replace_extension(".pdb");

				if (FileSystem::Exists(pdbPath))
				{
					UniqueBuffer pdbFileData = FileSystem::ReadBinary(pdbPath);

					mono_debug_open_image_from_memory(image, pdbFileData.As<const mono_byte>(), pdbFileData.Size());

					VX_CONSOLE_LOG_INFO("PDB Loaded : {}", pdbPath);
				}
			}

			std::string pathString = filepath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			return assembly;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
				VX_CONSOLE_LOG_TRACE("{}.{}", nameSpace, name);
			}
		}

		static std::vector<MonoAssemblyTypeInfo> GetAssemblyTypeInfo(MonoAssembly* assembly)
		{
			std::vector<MonoAssemblyTypeInfo> result;

			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

				if (name[0] == '<' || !monoClass)
					continue;

				int fieldCount = mono_class_num_fields(monoClass);

				result.emplace_back(nameSpace, name, fieldCount);
			}

			return result;
		}

		static ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);
			if (it == s_ScriptFieldTypeMap.end())
				return ScriptFieldType::None;

			return it->second;
		}

	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;
			
		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path AppAssemblyFilepath;

		SharedRef<ScriptClass> EntityClass = nullptr;

		UniqueRef<filewatch::FileWatch<std::string>> AppAssemblyFilewatcher = nullptr;
		bool AssemblyReloadPending = false;
		bool DebuggingEnabled = false;

		SharedRef<AudioSource> AppAssemblyReloadSound = nullptr;

		std::unordered_map<std::string, SharedRef<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, SharedRef<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// Runtime
		Scene* ContextScene = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (!s_Data->AssemblyReloadPending && changeType == filewatch::Event::modified)
		{
			// Add reload to main thread queue

			s_Data->AssemblyReloadPending = true;

			Application::Get().SubmitToMainThread([]()
			{
				s_Data->AppAssemblyFilewatcher.reset();

				ScriptEngine::ReloadAssembly();
				s_Data->AppAssemblyReloadSound->Play();
			});
		}
	}

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		SharedRef<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();
		s_Data->DebuggingEnabled = projectProps.ScriptingProps.EnableMonoDebugging;

		InitMono();
		ScriptRegistry::RegisterMethods();

		std::filesystem::path coreAssemblyPath = "Resources/Scripts/Vortex-ScriptCore.dll";
		bool status = LoadAssembly(coreAssemblyPath);

		if (!status)
		{
			VX_CONSOLE_LOG_ERROR("Failed to load Vortex-ScriptCore from path: {}", coreAssemblyPath);
			return;
		}

		std::filesystem::path appAssemblyPath = Project::GetAssetFileSystemPath(projectProps.ScriptingProps.ScriptBinaryPath);
		status = LoadAppAssembly(appAssemblyPath);
		
		if (!status)
		{
			VX_CONSOLE_LOG_ERROR("Failed to load App Assembly from path: {}", appAssemblyPath);
			return;
		}

		LoadAssemblyClasses();

		ScriptRegistry::RegisterComponents();

		s_Data->EntityClass = CreateShared<ScriptClass>("Vortex", "Entity", true);
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

		SharedRef<Project> activeProject = Project::GetActive();
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
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_Data->DebuggingEnabled);

		if (s_Data->CoreAssembly == nullptr)
			return false;

		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);

		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssemblyFilepath = filepath;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath, s_Data->DebuggingEnabled);

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

		s_Data->EntityClass = CreateShared<ScriptClass>("Vortex", "Entity", true);
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

		VX_CORE_ASSERT(!ScriptEngine::EntityInstanceExists(entityUUID), "Instance was already found with UUID!");
		VX_CORE_ASSERT(ScriptEngine::EntityClassExists(scriptComponent.ClassName), "Class was not found in Entity Classes Map!");

		SharedRef<ScriptClass> scriptClass = GetEntityClass(scriptComponent.ClassName);
		SharedRef<ScriptInstance> instance = CreateShared<ScriptInstance>(scriptClass, entity);
		s_Data->EntityInstances[entityUUID] = instance;

		// Copy field values
		auto it = s_Data->EntityScriptFields.find(entityUUID);

		if (it == s_Data->EntityScriptFields.end())
			return;

		const ScriptFieldMap& fields = it->second;

		for (const auto& [name, fieldInstance] : fields)
		{
			instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
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

		SharedRef<ScriptInstance> instance = GetEntityScriptInstance(entityUUID);
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

	SharedRef<ScriptClass> ScriptEngine::GetCoreEntityClass()
	{
		return s_Data->EntityClass;
	}

	SharedRef<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID uuid)
	{
		auto it = s_Data->EntityInstances.find(uuid);

		if (it != s_Data->EntityInstances.end())
			return it->second;

		return nullptr;
	}

	SharedRef<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		auto it = s_Data->EntityClasses.find(name);

		if (it != s_Data->EntityClasses.end())
			return it->second;

		return nullptr;
	}

	std::unordered_map<std::string, SharedRef<ScriptClass>> ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
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
		return Utils::GetAssemblyTypeInfo(s_Data->CoreAssembly);
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

			SharedRef<ScriptClass> scriptClass = CreateShared<ScriptClass>(nameSpace, className);
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
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);

					if (displayClassNames)
						VX_CONSOLE_LOG_INFO("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_Fields[fieldName] = ScriptField(fieldType, fieldName, classField);
				}
			}
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		return ScriptUtils::InstantiateClass(monoClass);
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return ScriptUtils::GetManagedMethodFromName(m_MonoClass, name.c_str(), parameterCount);
	}

	ScriptInstance::ScriptInstance(SharedRef<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_ManagedMethods[ManagedMethod::OnAwake] = m_ScriptClass->GetMethod("OnAwake", 0);
		m_ManagedMethods[ManagedMethod::OnCreate] = m_ScriptClass->GetMethod("OnCreate", 0);
		m_ManagedMethods[ManagedMethod::OnUpdateDelta] = m_ScriptClass->GetMethod("OnUpdate", 1);
		m_ManagedMethods[ManagedMethod::OnUpdate] = m_ScriptClass->GetMethod("OnUpdate", 0);
		m_ManagedMethods[ManagedMethod::OnDestroy] = m_ScriptClass->GetMethod("OnDestroy", 0);
		m_ManagedMethods[ManagedMethod::OnCollisionEnter] = m_ScriptClass->GetMethod("OnCollisionEnter", 1);
		m_ManagedMethods[ManagedMethod::OnCollisionExit] = m_ScriptClass->GetMethod("OnCollisionExit", 1);
		m_ManagedMethods[ManagedMethod::OnTriggerEnter] = m_ScriptClass->GetMethod("OnTriggerEnter", 1);
		m_ManagedMethods[ManagedMethod::OnTriggerExit] = m_ScriptClass->GetMethod("OnTriggerExit", 1);
		m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected] = m_ScriptClass->GetMethod("OnFixedJointDisconnected", 2);
		m_ManagedMethods[ManagedMethod::OnRaycastCollision] = m_ScriptClass->GetMethod("OnRaycastCollision", 0);
		m_ManagedMethods[ManagedMethod::OnEnabled] = m_ScriptClass->GetMethod("OnEnabled", 0);
		m_ManagedMethods[ManagedMethod::OnDisabled] = m_ScriptClass->GetMethod("OnDisabled", 0);
		m_ManagedMethods[ManagedMethod::OnGui] = m_ScriptClass->GetMethod("OnGui", 0);

		// Call C# Entity constructor
		ScriptEngine::ConstructEntityRuntime(entity.GetUUID(), m_Instance);
	}

	void ScriptInstance::InvokeOnAwake()
	{
		if (!m_ManagedMethods[ManagedMethod::OnAwake])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnAwake]);
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (!m_ManagedMethods[ManagedMethod::OnCreate])
			return;
		
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCreate]);
	}

	void ScriptInstance::InvokeOnUpdate(float delta)
	{
		MonoMethod* onUpdateMethod = nullptr;

		if (m_ManagedMethods[ManagedMethod::OnUpdateDelta])
		{
			void* param = &delta;
			ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnUpdateDelta], &param);
		}
		else if (m_ManagedMethods[ManagedMethod::OnUpdate])
		{
			ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnUpdate]);
		}
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (!m_ManagedMethods[ManagedMethod::OnDestroy])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnDestroy]);
	}

	void ScriptInstance::InvokeOnCollisionEnter(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnCollisionEnter])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCollisionEnter], &param);
	}

	void ScriptInstance::InvokeOnCollisionExit(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnCollisionExit])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnCollisionExit], &param);
	}

	void ScriptInstance::InvokeOnTriggerEnter(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnTriggerEnter])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnTriggerEnter], &param);
	}

	void ScriptInstance::InvokeOnTriggerExit(Collision& collision)
	{
		if (!m_ManagedMethods[ManagedMethod::OnTriggerExit])
			return;

		void* param = &collision;
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnTriggerExit], &param);
	}

	void ScriptInstance::InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque)
	{
		if (!m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected])
			return;

		void* params[] = { (void*)&forceAndTorque.first, (void*)&forceAndTorque.second };
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnFixedJointDisconnected], params);
	}

	void ScriptInstance::InvokeOnRaycastCollision()
	{
		if (!m_ManagedMethods[ManagedMethod::OnRaycastCollision])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnRaycastCollision]);
	}

	void ScriptInstance::InvokeOnEnabled()
	{
		if (!m_ManagedMethods[ManagedMethod::OnEnabled])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnEnabled]);
	}

	void ScriptInstance::InvokeOnDisabled()
	{
		if (!m_ManagedMethods[ManagedMethod::OnDisabled])
			return;

		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnDisabled]);
	}

	void ScriptInstance::InvokeOnGui()
	{
		if (!m_ManagedMethods[ManagedMethod::OnGui])
			return;
		
		ScriptUtils::InvokeMethod(m_Instance, m_ManagedMethods[ManagedMethod::OnGui]);
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& fieldName, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(fieldName);

		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& fieldName, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(fieldName);

		if (it == fields.end())
			return false;

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}

}

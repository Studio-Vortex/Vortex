#include "sppch.h"
#include "ScriptEngine.h"

#include "Sparky/Core/Application.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Audio/AudioSource.h"
#include "Sparky/Scripting/ScriptRegistry.h"
#include "Sparky/Scripting/ScriptEngine.h"
#include "Sparky/Utils/PlatformUtils.h"
#include "Sparky/Debug/Instrumentor.h"

#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/attrdefs.h>
#include <mono/metadata/class.h>
#include <mono/jit/jit.h>

#include <Filewatch.hpp>

namespace Sparky {

	static constexpr const char* APP_ASSEMBLY_PATH = "C:/dev/Sparky_Game_Engine/Sparky/Sparky-Editor/SandboxProject/Assets/Scripts/Binaries/Sandbox.dll";
	
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
		{ "Sparky.Vector2", ScriptFieldType::Vector2 },
		{ "Sparky.Vector3", ScriptFieldType::Vector3 },
		{ "Sparky.Vector4", ScriptFieldType::Vector4 },
		{ "Sparky.Entity",  ScriptFieldType::Entity  },
	};

	namespace Utils {

		// TODO: Move to FileSystem class
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				// Failed to open the file
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint64_t size = end - stream.tellg();

			if (size == 0)
			{
				// File is empty
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read((char*)buffer, size);
			stream.close();

			*outSize = (uint32_t)size;
			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& filepath)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(filepath, &fileSize);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				SP_CORE_ERROR("Mono Assembly Error: {}", errorMessage);
				return nullptr;
			}

			std::string pathString = filepath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			delete[] fileData;

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
				SP_CORE_TRACE("{}.{}", nameSpace, name);
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

		ScriptClass EntityClass;

		UniqueRef<filewatch::FileWatch<std::string>> AppAssemblyFilewatcher;
		bool AssemblyReloadPending = false;

		SharedRef<AudioSource> CompileSound;

		std::unordered_map<std::string, SharedRef<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, SharedRef<ScriptInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// Runtime
		Scene* ContextScene = nullptr;
	};

	static ScriptEngineData* s_Data = nullptr;

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		InitMono();
		ScriptRegistry::RegisterMethods();

		LoadAssembly("Resources/Scripts/Sparky-ScriptCore.dll");
		LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");
		LoadAssemblyClasses();

		ScriptRegistry::RegisterComponents();

		s_Data->EntityClass = ScriptClass("Sparky", "Entity", true);
		s_Data->CompileSound = CreateShared<AudioSource>("Resources/Sounds/Compile.wav");
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete s_Data;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		MonoDomain* rootDomain = mono_jit_init("SparkyJITRuntime");
		SP_CORE_ASSERT(rootDomain, "Root Domain was null pointer!");

		// Store the root domain pointer
		s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		// Create an App Domain
		char name[20] = "SparkyScriptRuntime";
		s_Data->AppDomain = mono_domain_create_appdomain(name, nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		// Move this
		s_Data->CoreAssemblyFilepath = filepath;
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		//Utils::PrintAssemblyTypes(s_Data->CoreAssembly);
	}

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event changeType)
	{
		if (!s_Data->AssemblyReloadPending && changeType == filewatch::Event::modified)
		{
			// Add reload to main thread queue

			s_Data->AssemblyReloadPending = true;

			Application::Get().SubmitToMainThread([]()
			{
				s_Data->AppAssemblyFilewatcher.reset();
				FileSystem::LaunchApplication("CopyMonoAssembly.bat", "");
				
				using namespace std::chrono_literals;
				std::this_thread::sleep_for(500ms);
				
				ScriptEngine::ReloadAssembly();
				s_Data->CompileSound->Play();
			});
		}
	}

	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		// Move this
		s_Data->AppAssemblyFilepath = filepath;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		//Utils::PrintAssemblyTypes(s_Data->AppAssembly);

		// TODO HARDCODED FILEPATH
		s_Data->AppAssemblyFilewatcher = CreateUnique<filewatch::FileWatch<std::string>>(APP_ASSEMBLY_PATH, OnAppAssemblyFileSystemEvent);
		s_Data->AssemblyReloadPending = false;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);

		LoadAssembly(s_Data->CoreAssemblyFilepath);
		LoadAppAssembly(s_Data->AppAssemblyFilepath);

		LoadAssemblyClasses();

		ScriptRegistry::RegisterComponents();

		s_Data->EntityClass = ScriptClass("Sparky", "Entity", true);
	}

	void ScriptEngine::OnRuntimeStart(Scene* contextScene)
	{
		s_Data->ContextScene = contextScene;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->ContextScene = nullptr;

		// TODO: REMOVE THIS
		if (s_Data->EntityInstances.size() > INT_MAX)
			return;

		if (!s_Data->EntityInstances.empty())
			s_Data->EntityInstances.clear();
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		auto& scriptComponent = entity.GetComponent<ScriptComponent>();

		if (EntityClassExists(scriptComponent.ClassName))
		{
			UUID uuid = entity.GetUUID();

			SharedRef<ScriptInstance> instance = CreateShared<ScriptInstance>(s_Data->EntityClasses[scriptComponent.ClassName], entity);
			s_Data->EntityInstances[uuid] = instance;

			// Copy field values
			auto it = s_Data->EntityScriptFields.find(uuid);
			if (it != s_Data->EntityScriptFields.end())
			{
				const ScriptFieldMap& fields = it->second;
				for (const auto& [name, fieldInstance] : fields)
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, TimeStep delta)
	{
		UUID uuid = entity.GetUUID();
		auto it = s_Data->EntityInstances.find(uuid);

		SP_CORE_ASSERT(it != s_Data->EntityInstances.end(), "Instance was not found in Entity Instance Map!");

		it->second->InvokeOnUpdate(delta);
	}

	void ScriptEngine::OnDestroyEntity(Entity entity)
	{
		UUID uuid = entity.GetUUID();
		auto it = s_Data->EntityInstances.find(uuid);

		SP_CORE_ASSERT(it != s_Data->EntityInstances.end(), "Instance was not found in Entity Instance Map!");

		it->second->InvokeOnDestroy();

		// Remove the instance from the map because it is no longer an active instance of a class
		s_Data->EntityInstances.erase(it);
	}

	void ScriptEngine::OnCollisionEntity(Entity entity)
	{
		UUID uuid = entity.GetUUID();
		auto it = s_Data->EntityInstances.find(uuid);

		SP_CORE_ASSERT(it != s_Data->EntityInstances.end(), "Instance was not found in Entity Instance Map!");

		it->second->InvokeOnCollision();
	}

	void ScriptEngine::OnGuiEntity(Entity entity)
	{
		UUID uuid = entity.GetUUID();
		auto it = s_Data->EntityInstances.find(uuid);

		SP_CORE_ASSERT(it != s_Data->EntityInstances.end(), "Instance was not found in Entity Instance Map!");

		it->second->InvokeOnGui();
	}

	SharedRef<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID uuid)
	{
		auto it = s_Data->EntityInstances.find(uuid);

		if (it != s_Data->EntityInstances.end())
			return it->second;
		else
			return nullptr;
	}

	SharedRef<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		auto it = s_Data->EntityClasses.find(name);

		if (it != s_Data->EntityClasses.end())
			return it->second;
		else
			return nullptr;
	}

	std::unordered_map<std::string, SharedRef<ScriptClass>> ScriptEngine::GetClasses()
	{
		return s_Data->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		SP_CORE_ASSERT(entity, "Entity was invalid!");

		return s_Data->EntityScriptFields[entity.GetUUID()];
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		auto it = s_Data->EntityInstances.find(uuid);

		SP_CORE_ASSERT(it != s_Data->EntityInstances.end(), "Entity was not found in Entity Instance Map!");

		return it->second->GetManagedObject();
	}

	std::vector<MonoAssemblyTypeInfo> Sparky::ScriptEngine::GetCoreAssemblyTypeInfo()
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

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Sparky", "Entity");

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

			if (monoClass == entityClass)
				continue;

			bool isEntityClass = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntityClass)
				continue;

			SharedRef<ScriptClass> scriptClass = CreateShared<ScriptClass>(nameSpace, className);
			s_Data->EntityClasses[fullName] = scriptClass;

			int fieldCount = mono_class_num_fields(monoClass);
			SP_CORE_WARN("{} has {} fields: ", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* classField = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(classField);
				uint32_t flags = mono_field_get_flags(classField);

				if (flags & MONO_FIELD_ATTR_PUBLIC)
				{
					MonoType* type = mono_field_get_type(classField);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					SP_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_Fields[fieldName] = { fieldType, fieldName, classField };
				}
			}
		}
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
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
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		return mono_runtime_invoke(method, instance, params, nullptr);
	}

	ScriptInstance::ScriptInstance(SharedRef<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = m_ScriptClass->Instantiate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateFunc    = m_ScriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateFunc    = m_ScriptClass->GetMethod("OnUpdate", 1);
		m_OnDestroyFunc   = m_ScriptClass->GetMethod("OnDestroy", 0);
		m_OnCollisionFunc = m_ScriptClass->GetMethod("OnCollision", 0);
		m_OnGuiFunc       = m_ScriptClass->GetMethod("OnGui", 0);

		// Call Entity constructor
		{
			UUID entitytUUID = entity.GetUUID();
			void* param = &entitytUUID;
			scriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		if (m_OnCreateFunc)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateFunc);
	}

	void ScriptInstance::InvokeOnUpdate(float delta)
	{
		if (m_OnUpdateFunc)
		{
			void* param = &delta;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateFunc, &param);
		}
	}

	void ScriptInstance::InvokeOnDestroy()
	{
		if (m_OnDestroyFunc)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnDestroyFunc);
	}

	void ScriptInstance::InvokeOnCollision()
	{
		if (m_OnCollisionFunc)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionFunc);
	}

	void ScriptInstance::InvokeOnGui()
	{
		if (m_OnGuiFunc)
			m_ScriptClass->InvokeMethod(m_Instance, m_OnGuiFunc);
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

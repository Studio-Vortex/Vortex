#pragma once

#include "Sparky/Scene/Scene.h"

#include <filesystem>
#include <string>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoClassField MonoClassField;
}

namespace Sparky {

	enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Short, Int, Long,
		Byte, UShort, UInt, ULong,
		Vector2, Vector3, Vector4,
		Entity,
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;

		MonoClassField* ClassField;
	};

	struct ScriptFieldInstance
	{
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

		template <typename T>
		T GetValue()
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_Buffer;
		}

		template <typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(T));
		}

	private:
		uint8_t m_Buffer[16];

	private:
		friend class ScriptEngine;
		friend class ScriptInstance;
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

		inline std::map<std::string, ScriptField> GetFields() { return m_Fields; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;

	private:
		friend class ScriptEngine;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(SharedRef<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnUpdate(float delta);
		void InvokeOnDestroy();
		void InvokeOnCollision();
		void InvokeOnGui();

		inline SharedRef<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template <typename T>
		T GetFieldValue(const std::string& fieldName)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			bool success = GetFieldValueInternal(fieldName, s_FieldValueBuffer);
			if (!success)
				return T();

			return *(T*)s_FieldValueBuffer;
		}

		template <typename T>
		void SetFieldValue(const std::string& fieldName, T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			SetFieldValueInternal(fieldName, &value);
		}

		MonoObject* GetManagedObject() const { return m_Instance; }

	private:
		bool GetFieldValueInternal(const std::string& fieldName, void* buffer);
		bool SetFieldValueInternal(const std::string& fieldName, const void* value);

	private:
		SharedRef<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;

		MonoMethod* m_OnCreateFunc = nullptr;
		MonoMethod* m_OnUpdateFunc = nullptr;
		MonoMethod* m_OnDestroyFunc = nullptr;
		MonoMethod* m_OnCollisionFunc = nullptr;
		MonoMethod* m_OnGuiFunc = nullptr;

		inline static char s_FieldValueBuffer[16];

	private:
		friend class ScriptEngine;
		friend struct ScriptFieldInstance;
	};

	struct MonoAssemblyTypeInfo
	{
		const char* Namespace;
		const char* Name;
		uint32_t FieldCount;
	};

	// Forward declaration
	class TimeStep;

	class ScriptEngine
	{
	public:
		ScriptEngine() = delete;
		ScriptEngine(const ScriptEngine&) = delete;

		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadAppAssembly(const std::filesystem::path& filepath);
		static void ReloadAssembly();

		static void OnRuntimeStart(Scene* contextScene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& fullClassName);

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, TimeStep delta);
		static void OnDestroyEntity(Entity entity);
		static void OnCollisionEntity(Entity entity);
		static void OnGuiEntity(Entity entity);

		static Scene* GetContextScene();
		static MonoImage* GetCoreAssemblyImage();

		static SharedRef<ScriptInstance> GetEntityScriptInstance(UUID uuid);

		static SharedRef<ScriptClass> GetEntityClass(const std::string& name);
		static std::unordered_map<std::string, SharedRef<ScriptClass>> GetClasses();
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

		static MonoObject* GetManagedInstance(UUID uuid);

		static std::vector<MonoAssemblyTypeInfo> GetCoreAssemblyTypeInfo();

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses();

	private:
		friend class ScriptClass;
		friend class ScriptRegistry;
	};

	namespace Utils {

		inline const char* ScriptFieldTypeToString(ScriptFieldType type)
		{
			switch (type)
			{
				case ScriptFieldType::None:    return "None";
				case ScriptFieldType::Float:   return "Float";
				case ScriptFieldType::Double:  return "Double";
				case ScriptFieldType::Bool:    return "Bool";
				case ScriptFieldType::Char:    return "Char";
				case ScriptFieldType::Short:   return "Short";
				case ScriptFieldType::Int:     return "Int";
				case ScriptFieldType::Long:    return "Long";
				case ScriptFieldType::Byte:    return "Byte";
				case ScriptFieldType::UShort:  return "UShort";
				case ScriptFieldType::UInt:    return "UInt";
				case ScriptFieldType::ULong:   return "ULong";
				case ScriptFieldType::Vector2: return "Vector2";
				case ScriptFieldType::Vector3: return "Vector3";
				case ScriptFieldType::Vector4: return "Vector4";
				case ScriptFieldType::Entity:  return "Entity";
			}

			SP_CORE_ASSERT(false, "Unknown Script Field Type!");
			return "None";
		}

		inline ScriptFieldType StringToScriptFieldType(std::string_view fieldType)
		{
			if (fieldType == "None")    return ScriptFieldType::None;
			if (fieldType == "Float")   return ScriptFieldType::Float;
			if (fieldType == "Double")  return ScriptFieldType::Double;
			if (fieldType == "Bool")    return ScriptFieldType::Bool;
			if (fieldType == "Char")    return ScriptFieldType::Char;
			if (fieldType == "Short")   return ScriptFieldType::Short;
			if (fieldType == "Int")     return ScriptFieldType::Int;
			if (fieldType == "Long")    return ScriptFieldType::Long;
			if (fieldType == "Byte")    return ScriptFieldType::Byte;
			if (fieldType == "UShort")  return ScriptFieldType::UShort;
			if (fieldType == "UInt")    return ScriptFieldType::UInt;
			if (fieldType == "ULong")   return ScriptFieldType::ULong;
			if (fieldType == "Vector2") return ScriptFieldType::Vector2;
			if (fieldType == "Vector3") return ScriptFieldType::Vector3;
			if (fieldType == "Vector4") return ScriptFieldType::Vector4;
			if (fieldType == "Entity")  return ScriptFieldType::Entity;

			SP_CORE_ASSERT(false, "Unknown Script Field Type!");
			return ScriptFieldType::None;
		}

	}

}

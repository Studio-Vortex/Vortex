#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Asset/Asset.h"

#include <filesystem>
#include <string>

extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoClassField MonoClassField;
}

namespace Vortex {

	// Forward declarations
	class TimeStep;
	struct Collision;

	enum class VORTEX_API ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Short, Int, Long,
		Byte, UShort, UInt, ULong,
		Vector2, Vector3, Vector4,
		Color3, Color4,
		Entity,
	};

	struct VORTEX_API ScriptField
	{
		ScriptFieldType Type = ScriptFieldType::None;
		std::string Name = "";

		MonoClassField* ClassField = nullptr;
	};

	struct VORTEX_API ScriptFieldInstance
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

	class VORTEX_API ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);
		~ScriptClass() = default;

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);

		inline std::map<std::string, ScriptField> GetFields() { return m_Fields; }

		inline MonoClass* GetMonoClass() const { return m_MonoClass; }

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;

	private:
		friend class ScriptEngine;
	};

	class VORTEX_API ScriptInstance
	{
	public:
		ScriptInstance() = default;
		ScriptInstance(SharedRef<ScriptClass> scriptClass, Entity entity);
		~ScriptInstance() = default;

		void InvokeOnAwake();
		void InvokeOnCreate();
		void InvokeOnUpdate(float delta);
		void InvokeOnDestroy();
		void InvokeOnCollisionEnter(Collision& collision);
		void InvokeOnCollisionExit(Collision& collision);
		void InvokeOnTriggerEnter(Collision& collision);
		void InvokeOnTriggerExit(Collision& collision);
		void InvokeOnFixedJointDisconnected(const std::pair<Math::vec3, Math::vec3>& forceAndTorque);
		void InvokeOnRaycastCollision();
		void InvokeOnEnabled();
		void InvokeOnDisabled();
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

		enum class ManagedMethod
		{
			OnAwake,
			OnCreate,
			OnUpdateDelta,
			OnUpdate,
			OnDestroy,
			OnCollisionEnter,
			OnCollisionExit,
			OnTriggerEnter,
			OnTriggerExit,
			OnFixedJointDisconnected,
			OnRaycastCollision,
			OnEnabled,
			OnDisabled,
			OnGui,
		};

		MonoObject* m_Instance = nullptr;

		std::unordered_map<ManagedMethod, MonoMethod*> m_ManagedMethods;

		inline static char s_FieldValueBuffer[16];

#ifdef VX_DEBUG
		std::string m_DebugName = "";
#endif

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

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	class VORTEX_API ScriptEngine
	{
	public:
		ScriptEngine() = delete;
		ScriptEngine(const ScriptEngine&) = delete;

		static void Init();
		static void Shutdown();

		static bool LoadAssembly(const std::filesystem::path& filepath);
		static bool LoadAppAssembly(const std::filesystem::path& filepath);
		static void ReloadAssembly();

		static void OnRuntimeStart(Scene* contextScene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& fullyQualifiedClassName);
		static bool EntityInstanceExists(UUID entityUUID);
		static void ConstructEntityRuntime(UUID entityUUID, MonoObject* instance);
		static void CreateEntityScriptInstanceRuntime(Entity entity);

		static void OnAwakeEntity(Entity entity);
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, TimeStep delta);
		static void OnDestroyEntity(Entity entity);
		static void OnCollisionEnterEntity(Entity entity, Collision& collision);
		static void OnCollisionExitEntity(Entity entity, Collision& collision);
		static void OnTriggerEnterEntity(Entity entity, Collision& collision);
		static void OnTriggerExitEntity(Entity entity, Collision& collision);
		static void OnFixedJointDisconnected(Entity entity, const std::pair<Math::vec3, Math::vec3>& forceAndTorque);
		static void OnRaycastCollisionEntity(Entity entity);
		static void OnEnabled(Entity entity);
		static void OnDisabled(Entity entity);
		static void OnGuiEntity(Entity entity);

		static SharedRef<ScriptClass> GetCoreEntityClass();

		static Scene* GetContextScene();
		static MonoImage* GetCoreAssemblyImage();
		static MonoDomain* GetAppDomain();

		static void DuplicateScriptInstance(Entity entity, Entity targetEntity);

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
		static void LoadAssemblyClasses(bool displayClassNames = false);

	private:
		friend class ScriptClass;
		friend class ScriptRegistry;
	};

	namespace Utils {

		const char* ScriptFieldTypeToString(ScriptFieldType type);
		ScriptFieldType StringToScriptFieldType(std::string_view fieldType);

	}

}

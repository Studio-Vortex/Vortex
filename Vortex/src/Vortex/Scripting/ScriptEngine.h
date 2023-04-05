#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Asset/Asset.h"

#include "Vortex/Scripting/MonoAssemblyTypeInfo.h"
#include "Vortex/Scripting/ScriptFieldInstance.h"
#include "Vortex/Scripting/ScriptFieldTypes.h"
#include "Vortex/Scripting/ScriptInstance.h"
#include "Vortex/Scripting/ScriptField.h"
#include "Vortex/Scripting/ScriptClass.h"

#include "Vortex/Utils/FileSystem.h"

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

	class TimeStep;
	struct Collision;

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

		static SharedReference<ScriptClass> GetCoreEntityClass();

		static Scene* GetContextScene();
		static MonoImage* GetCoreAssemblyImage();
		static MonoDomain* GetAppDomain();
		static MonoImage* GetAppAssemblyImage();

		static void DuplicateScriptInstance(Entity entity, Entity targetEntity);

		static SharedReference<ScriptInstance> GetEntityScriptInstance(UUID uuid);

		static SharedReference<ScriptClass> GetEntityClass(const std::string& name);
		static std::unordered_map<std::string, SharedReference<ScriptClass>> GetClasses();
		static const ScriptFieldMap& GetScriptFieldMap(Entity entity);
		static ScriptFieldMap& GetMutableScriptFieldMap(Entity entity);

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

}

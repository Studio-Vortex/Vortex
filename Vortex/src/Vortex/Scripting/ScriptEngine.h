#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Asset/Asset.h"

#include "Vortex/Scene/Scene.h"

#include "Vortex/Scripting/RuntimeMethodArgument.h"
#include "Vortex/Scripting/MonoAssemblyTypeInfo.h"
#include "Vortex/Scripting/ScriptFieldInstance.h"
#include "Vortex/Scripting/ScriptFieldTypes.h"
#include "Vortex/Scripting/ManagedMethods.h"
#include "Vortex/Scripting/ScriptField.h"

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

	class ScriptClass;
	class ScriptInstance;

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
		static void EntityConstructorRuntime(UUID entityUUID, MonoObject* instance);
		static void RT_CreateEntityScriptInstance(Entity entity);

		static bool Invoke(const std::string& methodName, Entity entity, const std::vector<RuntimeMethodArgument>& argumentList);
		static bool Invoke(ManagedMethod method, Entity entity, const std::vector<RuntimeMethodArgument>& argumentList = {});

		static SharedReference<ScriptClass> GetCoreEntityClass();

		static Scene* GetContextScene();
		static MonoImage* GetCoreAssemblyImage();
		static MonoDomain* GetAppDomain();
		static MonoImage* GetAppAssemblyImage();

		static void RuntimeInstantiateEntity(Entity entity);

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

		static void LoadAssemblyClasses(bool displayClasses = false);
	};

}

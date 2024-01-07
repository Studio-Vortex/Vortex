#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Asset/Asset.h"

#include "Vortex/Scene/Scene.h"

#include "Vortex/Scripting/RuntimeMethodArgument.h"
#include "Vortex/Scripting/ScriptAssemblyTypedefInfo.h"
#include "Vortex/Scripting/ScriptFieldInstance.h"
#include "Vortex/Scripting/ScriptFieldTypes.h"
#include "Vortex/Scripting/ScriptMethods.h"
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

	using VORTEX_API ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

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

		static void OnRuntimeStart(Scene* context);
		static void OnRuntimeStop();

		static bool ScriptClassExists(const std::string& className);
		static bool IsScriptClassValid(Actor actor);
		static bool IsScriptComponentEnabled(Actor actor);

		static bool ScriptInstanceExists(UUID actorUUID);
		static bool ScriptInstanceHasMethod(Actor actor, ScriptMethod method);

		static void RT_ActorConstructor(UUID actorUUID, MonoObject* instance);
		static void RT_CreateActorScriptInstance(Actor actor);
		static void RT_InstantiateActor(Actor actor);

		static bool Invoke(Actor actor, const std::string& methodName, const std::vector<RuntimeMethodArgument>& argumentList);
		static bool Invoke(Actor actor, ScriptMethod method, const std::vector<RuntimeMethodArgument>& argumentList = {});

		static SharedReference<ScriptClass> GetCoreActorClass();

		static Scene* GetContextScene();
		static MonoImage* GetScriptCoreAssemblyImage();
		static MonoDomain* GetAppDomain();
		static MonoImage* GetAppAssemblyImage();

		static SharedReference<ScriptInstance> GetScriptInstance(UUID uuid);

		static SharedReference<ScriptClass> GetScriptClass(const std::string& className);
		static std::unordered_map<std::string, SharedReference<ScriptClass>> GetScriptClasses();
		static const ScriptFieldMap& GetScriptFieldMap(Actor actor);
		static ScriptFieldMap& GetMutableScriptFieldMap(Actor actor);

		static MonoObject* TryGetManagedInstance(UUID uuid);

		static std::vector<ScriptAssemblyTypedefInfo> GetCoreAssemblyTypeInfo();

		static size_t GetScriptInstanceCount();

	private:
		static void InitMono();
		static void ShutdownMono();

		static void LoadAssemblyClasses(bool displayClasses = false);
	};

}

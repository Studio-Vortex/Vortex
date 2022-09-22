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
}

namespace Sparky {

	class ScriptClass
	{
	public:
		ScriptClass() = default;
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);

	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptInstance
	{
	public:
		ScriptInstance(SharedRef<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnUpdate(float delta);
	private:
		SharedRef<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateFunc = nullptr;
		MonoMethod* m_OnUpdateFunc = nullptr;
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

		static void OnRuntimeStart(Scene* contextScene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& fullClassName);

		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, TimeStep delta);

		static Scene* GetContextScene();
		static MonoImage* GetCoreAssemblyImage();

		static std::unordered_map<std::string, SharedRef<ScriptClass>> GetClasses();

	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses();

	private:
		friend class ScriptClass;
		friend class ScriptRegistry;
	};

}

#pragma once

namespace Vortex {

	class Entity;

	class ScriptRegistry
	{
	public:
		static void RegisterMethods();
		static void RegisterComponents();
		static void SetHoveredEntity(Entity entity);
		static void SetSceneStartTime(float startTime);
		static void SetActiveSceneName(const std::string& sceneName);
		static const char* GetSceneToBeLoaded();
		static void ResetSceneToBeLoaded();
	};

}

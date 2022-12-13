#pragma once

namespace Sparky {

	class Entity;

	class ScriptRegistry
	{
	public:
		static void RegisterMethods();
		static void RegisterComponents();
		static void SetHoveredEntity(Entity entity);
		static const char* GetSceneToBeLoaded();
		static void ResetSceneToBeLoaded();
	};

}

#pragma once

namespace Sparky {

	class ScriptRegistry
	{
	public:
		static void RegisterMethods();
		static void RegisterComponents();
		static const char* GetSceneToBeLoaded();
		static void ResetSceneToBeLoaded();
	};

}

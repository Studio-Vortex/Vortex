#pragma once

#include <Vortex.h>

namespace Vortex {

	class ScriptRegistryPanel
	{
	public:
		ScriptRegistryPanel() = default;
		~ScriptRegistryPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(const SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}

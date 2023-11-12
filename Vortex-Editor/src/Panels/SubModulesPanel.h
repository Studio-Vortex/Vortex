#pragma once

#include <Vortex.h>

namespace Vortex {

	class SubModulesPanel
	{
	public:
		SubModulesPanel() = default;
		~SubModulesPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedReference<Project>& project) {}
		void SetSceneContext(SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderModule(const SubModule& submodule);

	private:
		inline static bool s_ShowPanel = false;
	};

}

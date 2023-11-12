#pragma once

#include <Vortex.h>

namespace Vortex {

	class SystemManagerPanel
	{
	public:
		SystemManagerPanel() = default;
		~SystemManagerPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedReference<Project>& project) { }
		void SetSceneContext(SharedReference<Scene>& scene) { }
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}
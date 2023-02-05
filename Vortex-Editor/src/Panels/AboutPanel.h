#pragma once

#include <Vortex.h>

namespace Vortex {

	class AboutPanel
	{
	public:
		AboutPanel() = default;
		~AboutPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(SharedRef<Scene> scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;
	};

}

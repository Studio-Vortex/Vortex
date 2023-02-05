#pragma once

#include <Vortex.h>

namespace Vortex {

	class ProjectSettingsPanel
	{
	public:
		ProjectSettingsPanel(SharedRef<Project> project);
		~ProjectSettingsPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project);
		void SetSceneContext(SharedRef<Scene> scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_Properties;
	};

}

#pragma once

#include <Vortex.h>

namespace Vortex {

	class BuildSettingsPanel
	{
	public:
		BuildSettingsPanel(const SharedRef<Project>& project);
		~BuildSettingsPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedRef<Project> project) {}
		void SetSceneContext(SharedRef<Scene> scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_ProjectProperties;

		std::filesystem::path m_ProjectPath;
		std::filesystem::path m_StartupScene;
	};

}

#pragma once

#include <Vortex.h>
#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class ProjectSettingsPanel : public EditorPanel
	{
	public:
		ProjectSettingsPanel(SharedRef<Project> project);
		~ProjectSettingsPanel() override = default;

		void OnGuiRender() override;
		void SetProjectContext(SharedRef<Project> project) override;
		void SetSceneContext(SharedRef<Scene> scene) override {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_Properties;
	};

}

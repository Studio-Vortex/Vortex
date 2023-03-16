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
		void SetSceneContext(const SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderGeneralSettingsPanel();
		void RenderPhysicsSettingsPanel();
		void RenderScriptingSettingsPanel();
		void RenderEditorSettingsPanel();

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_ProjectProperties;
		std::vector<std::function<void()>> m_CallbackPanels;
	};

}

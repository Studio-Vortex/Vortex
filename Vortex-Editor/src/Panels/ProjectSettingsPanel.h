#pragma once

#include <Vortex.h>

namespace Vortex {

	class ProjectSettingsPanel
	{
	public:
		ProjectSettingsPanel(SharedReference<Project>& project);
		~ProjectSettingsPanel() = default;

		void OnGuiRender();
		void SetProjectContext(SharedReference<Project>& project);
		void SetSceneContext(SharedReference<Scene>& scene) {}
		bool& IsOpen() { return s_ShowPanel; }

	private:
		void RenderGeneralSettingsPanel();
		void RenderEditorSettingsPanel();
		void RenderGizmoSettingsPanel();
		void RenderNetworkSettingsPanel();
		void RenderPhysicsSettingsPanel();
		void RenderScriptingSettingsPanel();

	private:
		inline static bool s_ShowPanel = false;

	private:
		ProjectProperties& m_ProjectProperties;
		std::vector<std::function<void()>> m_CallbackPanels;
	};

}

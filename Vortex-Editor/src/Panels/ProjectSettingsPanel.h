#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class ProjectSettingsPanel : public EditorPanel
	{
	public:
		ProjectSettingsPanel(SharedReference<Project> project);
		~ProjectSettingsPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(ProjectSettings)

	private:
		void RenderGeneralSettingsPanel();
		void RenderEditorSettingsPanel();
		void RenderGizmoSettingsPanel();
		void RenderNetworkSettingsPanel();
		void RenderPhysicsSettingsPanel();
		void RenderScriptingSettingsPanel();

	private:
		ProjectProperties& m_ProjectProperties;
		std::vector<std::function<void()>> m_CallbackPanels;
	};

}

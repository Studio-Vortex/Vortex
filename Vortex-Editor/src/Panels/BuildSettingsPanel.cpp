#include "BuildSettingsPanel.h"

namespace Vortex {

	BuildSettingsPanel::BuildSettingsPanel(const SharedRef<Project>& project, const LaunchRuntimeFn& callback)
		: m_ProjectProperties(project->GetProperties()), m_LaunchRuntimeCallback(callback)
	{
		auto projectFilename = m_ProjectProperties.General.Name + ".vxproject";
		m_ProjectPath = Project::GetProjectDirectory() / std::filesystem::path(projectFilename);
		m_StartupScene = m_ProjectProperties.General.StartScene;
	}

	void BuildSettingsPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::Begin("Build Settings", &s_ShowPanel);

		UI::BeginPropertyGrid();

		std::string projectPath = m_ProjectPath.string();
		UI::Property("Project Location", projectPath, true);

		std::string startupScene = m_StartupScene.string();
		UI::Property("Startup Scene", startupScene, true);

		UI::EndPropertyGrid();

		// Accept Items from the content browser
		if (Gui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path filePath = std::filesystem::path(path);

				if (filePath.extension().string() == ".vortex")
				{
					auto relativePath = std::filesystem::relative(filePath, Project::GetAssetDirectory());
					m_ProjectProperties.General.StartScene = relativePath;
					m_StartupScene = relativePath;
				}
			}

			Gui::EndDragDropTarget();
		}

		if (UI::PropertyGridHeader("Window", false))
		{
			UI::BeginPropertyGrid();

			if (!m_ProjectProperties.BuildProps.Window.Maximized)
				UI::Property("Size", m_ProjectProperties.BuildProps.Window.Size);
			UI::Property("Maximized", m_ProjectProperties.BuildProps.Window.Maximized);
			UI::Property("Decorated", m_ProjectProperties.BuildProps.Window.Decorated);
			UI::Property("Resizeable", m_ProjectProperties.BuildProps.Window.Resizeable);

			UI::EndPropertyGrid();
			UI::EndTreeNode();
		}

		UI::ShiftCursorY(10.0f);

		if (Gui::Button("Run Detached Process"))
		{
			if (m_LaunchRuntimeCallback && !m_ProjectPath.empty())
			{
				m_LaunchRuntimeCallback(m_ProjectPath);
			}
		}

		Gui::End();
	}

}

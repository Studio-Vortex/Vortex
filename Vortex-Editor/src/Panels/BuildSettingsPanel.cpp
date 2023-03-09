#include "BuildSettingsPanel.h"

namespace Vortex {

	BuildSettingsPanel::BuildSettingsPanel(const SharedRef<Project>& project, const LaunchRuntimeFn& func)
		: m_ProjectProperties(project->GetProperties()), m_LaunchRuntimeFunc(func)
	{
		m_ProjectPath = Project::GetProjectFilepath();
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

		std::string startupSceneFullPath = m_StartupScene.string();
		size_t lastSlashPos = startupSceneFullPath.find_last_of("/\\");
		std::string filepathWithExtension = startupSceneFullPath.substr(lastSlashPos);
		std::string startupSceneFilename = FileSystem::RemoveFileExtension(filepathWithExtension);
		UI::Property("Startup Scene", startupSceneFilename, true);

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

		UI::Draw::Underline();
		for (uint32_t i = 0; i < 3; i++)
			Gui::Spacing();

		if (Gui::Button("Build"))
		{
			// TODO build asset pack here
		}

		Gui::SameLine();

		if (Gui::Button("Build and Run"))
		{
			m_LaunchRuntimeFunc(m_ProjectPath);
		}
		
		Gui::End();
	}

}

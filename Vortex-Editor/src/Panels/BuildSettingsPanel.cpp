#include "BuildSettingsPanel.h"

namespace Vortex {

	void BuildSettingsPanel::SetLaunchRuntimeCallback(const LaunchRuntimeFn& callback)
	{
		m_LaunchRuntimeCallback = callback;
		const auto& projectProps = Project::GetActive()->GetProperties();
		auto projectFilename = std::format("{}.vxproject", projectProps.General.Name);
		const auto& dirPath = Project::GetProjectDirectory();
		m_ProjectPath = dirPath / std::filesystem::path(projectFilename);
		m_StartupScene = projectProps.General.StartScene;
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
					auto& projectProps = Project::GetActive()->GetProperties();
					auto relativePath = std::filesystem::relative(filePath, Project::GetAssetDirectory());
					projectProps.General.StartScene = relativePath;
					m_StartupScene = relativePath;
				}
			}

			Gui::EndDragDropTarget();
		}

		if (Gui::Button("Build and Run"))
		{
			if (m_LaunchRuntimeCallback && !m_ProjectPath.empty())
				m_LaunchRuntimeCallback(m_ProjectPath);
		}

		Gui::End();
	}

}

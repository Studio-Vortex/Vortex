#include "BuildSettingsPanel.h"

namespace Sparky {

	void BuildSettingsPanel::SetContext(const LaunchRuntimeFn& callback)
	{
		m_LaunchRuntimeCallback = callback;
		const auto& projectProps = Project::GetActive()->GetProperties();
		auto projectFilename = std::format("{}.sproject", projectProps.General.Name);
		const auto& dirPath = Project::GetProjectDirectory();
		m_ProjectPath = dirPath / std::filesystem::path(projectFilename);
		m_StartupScene = projectProps.General.StartScene;
	}

	void BuildSettingsPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Build Settings", &s_ShowPanel);

			char buffer[256];
			if (m_ProjectPath.empty())
				memset(buffer, 0, sizeof(buffer));
			else
				memcpy(buffer, m_ProjectPath.string().c_str(), m_ProjectPath.string().size());
			buffer[m_ProjectPath.string().size()] = '\0';

			Gui::InputText("Project", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

			char buffer2[256];
			if (m_StartupScene.empty())
				memset(buffer2, 0, sizeof(buffer2));
			else
				memcpy(buffer2, m_StartupScene.string().c_str(), m_StartupScene.string().size());
			buffer2[m_StartupScene.string().size()] = '\0';

			Gui::InputText("Startup Scene", buffer2, sizeof(buffer2), ImGuiInputTextFlags_ReadOnly);

			// Accept Items from the content browser
			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;
					std::filesystem::path filePath = std::filesystem::path(path);

					if (filePath.extension().string() == ".sparky")
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

}

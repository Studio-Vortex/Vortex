#include "BuildSettingsPanel.h"

namespace Sparky {

	void BuildSettingsPanel::SetContext(const LaunchRuntimeFn& callback)
	{
		m_LaunchRuntimeCallback = callback;
		auto projectFilename = std::format("{}.sproject", Project::GetActive()->GetProperties().General.Name);
		m_ProjectPath = Project::GetProjectDirectory() / std::filesystem::path(projectFilename);
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
				memcpy(buffer, m_ProjectPath.string().c_str(), m_ProjectPath.string().length());

			Gui::InputText("Startup Scene", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

			if (Gui::Button("Build and Run"))
			{
				if (m_LaunchRuntimeCallback && !m_ProjectPath.empty())
					m_LaunchRuntimeCallback(m_ProjectPath);
			}

			Gui::End();
		}
	}

}

#include "BuildSettingsPanel.h"

namespace Sparky {
	
	extern const std::filesystem::path g_AssetPath;

	void BuildSettingsPanel::SetContext(const LaunchRuntimeFn& callback)
	{
		m_LaunchRuntimeCallback = callback;
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

			if (m_StartupPath.empty())
				memset(buffer, 0, sizeof(buffer));
			else
				memcpy(buffer, m_StartupPath.string().c_str(), m_StartupPath.string().length());

			Gui::InputText("Startup Scene", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

			if (Gui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = Gui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const wchar_t* path = (const wchar_t*)payload->Data;

					if ((std::filesystem::path(path).extension().string()) == ".sparky")
					{
						m_StartupPath = std::filesystem::path(g_AssetPath) / path;
					}
				}

				Gui::EndDragDropTarget();
			}

			if (Gui::Button("Build and Run"))
			{
				m_LaunchRuntimeCallback(m_StartupPath);
			}

			Gui::End();
		}
	}

}

#include "SettingsPanel.h"

namespace Sparky {

	bool SettingsPanel::s_ShowPanel = false;

	void SettingsPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::Begin("Settings", &s_ShowPanel);

			Gui::DragFloat("Editor FOV", &m_Settings.EditorCameraFOV, 0.0f, 4.0f, 120.0f);

			if (Gui::ColorEdit3("Clear Color", Math::ValuePtr(m_Settings.ClearColor)))
				RenderCommand::SetClearColor(m_Settings.ClearColor);
			Gui::Separator();

			Gui::Checkbox("Show physics colliders", &m_Settings.ShowColliders);
			float lineWidth = Renderer2D::GetLineWidth();
			if (Gui::DragFloat("Collider width", &lineWidth, 0.1f, 0.01f, 5.0f))
				Renderer2D::SetLineWidth(lineWidth);
			Gui::ColorEdit4("Collider color", Math::ValuePtr(m_Settings.ColliderColor));
			Gui::Separator();

			static bool wireframeMode = false;
			if (Gui::Checkbox("Show wireframe", &wireframeMode))
				RenderCommand::SetWireframe(wireframeMode);

			Gui::Separator();

			static bool vsync = true;
			if (Gui::Checkbox("Use VSync", &vsync))
				Application::Get().GetWindow().SetVSync(vsync);

			Gui::End();
		}
	}

}

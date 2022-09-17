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

			static bool showColliders = false;
			if (Gui::Checkbox("Show physics colliders", &showColliders))
				m_Settings.ShowColliders = showColliders;

			float lineWidth = Renderer2D::GetLineWidth();
			if (Gui::DragFloat("Collider width", &lineWidth, 0.1f, 0.01f, 5.0f))
				Renderer2D::SetLineWidth(lineWidth);

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

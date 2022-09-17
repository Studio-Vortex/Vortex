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

			static bool wireframeMode = false;
			if (Gui::Checkbox("Show wireframe", &wireframeMode))
				RenderCommand::SetWireframe(wireframeMode);

			float lineWidth = Renderer2D::GetLineWidth();
			if (Gui::DragFloat("Collider width", &lineWidth, 0.1f, 0.1f, 10.0f))
				Renderer2D::SetLineWidth(lineWidth);

			Gui::End();
		}
	}

}

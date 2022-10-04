#include "AboutPanel.h"

namespace Sparky {

	void AboutPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel)
		{
			Gui::SetNextWindowSizeConstraints(ImVec2{ 480, 280 }, ImVec2{ 600, 400 });
			Gui::Begin("About", &s_ShowPanel);
			
			Gui::PushFont(largeFont);
			Gui::Text("Sparky Engine");
			Gui::PopFont();

			Gui::Separator();
			Gui::Text("Sparky is an early-stage game engine for Windows");
			Gui::Separator();

			Gui::PushFont(boldFont);
			Gui::Text("Sparky Core Team");
			Gui::PopFont();

			Gui::End();
		}
	}

}

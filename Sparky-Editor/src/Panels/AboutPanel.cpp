#include "AboutPanel.h"

namespace Sparky {

	void AboutPanel::OnGuiRender(bool showDefault)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (s_ShowPanel || showDefault)
		{
			Gui::SetNextWindowSizeConstraints(ImVec2{ 480, 280 }, ImVec2{ 600, 400 });
			Gui::Begin("About", &s_ShowPanel);
			
			Gui::PushFont(largeFont);
			Gui::Text("Sparky Engine");
			Gui::PopFont();

			Gui::Separator();
			Gui::Text("Sparky is an early-stage game engine for Windows.");
			Gui::Text("For more info, visit: https://github.com/Jshuk-7/Sparky");
			Gui::Text("This software contains source code powered by NVIDIA Corporation.");
			Gui::Separator();

			Gui::PushFont(boldFont);
			Gui::Text("Sparky Core Team");
			Gui::PopFont();

			Gui::End();
		}
	}

}

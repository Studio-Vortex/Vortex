#include "AboutPanel.h"

namespace Vortex {

	void AboutPanel::OnGuiRender()
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::SetNextWindowSizeConstraints(ImVec2{ 480, 280 }, ImVec2{ 600, 400 });
		Gui::Begin("About", &s_ShowPanel);

		Gui::PushFont(largeFont);
		Gui::Text("Vortex Engine");
		Gui::PopFont();

		Gui::Separator();
		Gui::Text("Vortex is an early-stage game engine for Windows.");
		Gui::Text("For more info, visit: https://github.com/Jshuk-7/Vortex");
		Gui::Text("This software contains source code powered by NVIDIA Corporation.");
		Gui::Separator();

		Gui::PushFont(boldFont);
		Gui::Text("Vortex Core Team");
		Gui::PopFont();

		Gui::End();
	}

}

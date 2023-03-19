#include "PhysicsMaterialEditorPanel.h"

namespace Vortex {

	void PhysicsMaterialEditorPanel::OnGuiRender(Entity selectedEntity)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];
		auto largeFont = io.Fonts->Fonts[1];

		if (!s_ShowPanel)
			return;

		Gui::SetNextWindowSizeConstraints(ImVec2{ 480, 280 }, ImVec2{ 600, 400 });
		Gui::Begin("About", &s_ShowPanel);

		Gui::PushFont(largeFont);
		Gui::Text("Vortex Game Engine");
		Gui::PopFont();

		UI::Draw::Underline();
		Gui::Text("Vortex is an early-stage game engine and interactive application for Windows.");

		Gui::Spacing();

		Gui::Text("This software contains source code powered by NVIDIA Corporation.");
		UI::Draw::Underline();

		Gui::Spacing();
		
		if (Gui::Button("Go To Documentation"))
		{
			Platform::OpenURLInBrowser("https://jshuk-7.github.io/Vortex-Web");
		}

		Gui::Spacing();
		UI::Draw::Underline();

		Gui::PushFont(boldFont);
		Gui::Text("Vortex Core Team");
		Gui::PopFont();

		Gui::End();
	}

}

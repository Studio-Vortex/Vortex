#include "AboutPanel.h"

namespace Vortex {

	static constexpr const char* VORTEX_WEBSITE_URL = "https://jshuk-7.github.io/Vortex-Web";
	static constexpr const char* VORTEX_DOCS_URL = "https://jshuk-7.github.io/Vortex-Web/documentation.html";

	void AboutPanel::OnPanelAttach()
	{
		m_Contributers.push_back(Contributor{ "Jesse", ContributorType::LeadDev });
	}

	void AboutPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		const ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;

		Gui::SetNextWindowSizeConstraints(ImVec2{ 480, 280 }, ImVec2{ 600, 400 });
		Gui::Begin(m_PanelName.c_str(), &IsOpen, flags);

		UI::PushFont("Large");
		Gui::Text("Vortex Game Engine");
		UI::PopFont();

		UI::Draw::Underline();
		Gui::Text("Vortex is an early-stage game engine and interactive rendering application for Windows.");

		Gui::Spacing();

		Gui::Text("This software contains source code powered by NVIDIA Corporation.");
		UI::Draw::Underline();

		Gui::Spacing();
		
		if (Gui::Button("Go to Docs"))
		{
			Platform::OpenURLInBrowser(VORTEX_DOCS_URL);
		}
		Gui::SameLine();

		if (Gui::Button("Go to Website"))
		{
			Platform::OpenURLInBrowser(VORTEX_WEBSITE_URL);
		}

		Gui::Spacing();
		UI::Draw::Underline();

		UI::PushFont("Bold");
		Gui::Text("Vortex Core Team");
		UI::PopFont();

		RenderContributors();

		Gui::End();
	}

	void AboutPanel::RenderContributors() const
	{
		for (const auto& contributor : m_Contributers)
		{
			Gui::Bullet();
			Gui::SameLine();
			
			const std::string& name = contributor.Name;
			Gui::Text(name.c_str());
			Gui::SameLine();
			
			std::string role = "";
			switch (contributor.Type)
			{
				case ContributorType::Dev:     role = "Developer";      break;
				case ContributorType::LeadDev: role = "Lead Developer"; break;
			}
			
			Gui::Text("(%s)", role.c_str());
		}
	}

}

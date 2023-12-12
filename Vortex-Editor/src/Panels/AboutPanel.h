#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

#include <vector>

namespace Vortex {

	class AboutPanel : public EditorPanel
	{
	public:
		AboutPanel() = default;
		~AboutPanel() override = default;

		void OnPanelAttach() override;
		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(About)

	private:
		void RenderContributors() const;

	private:
		enum class ContributorType
		{
			Dev,
			LeadDev,
		};

		struct Contributor
		{
			std::string Name;
			ContributorType Type;
		};

	private:
		std::vector<Contributor> m_Contributers;
	};

}

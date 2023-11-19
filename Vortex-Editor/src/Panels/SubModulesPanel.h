#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class SubModulesPanel : public EditorPanel
	{
	public:
		SubModulesPanel() = default;
		~SubModulesPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(SubModules)

	private:
		void RenderModule(const SubModule& submodule);
	};

}

#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class PerformancePanel : public EditorPanel
	{
	public:
		~PerformancePanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(Performance)

	private:
		void DrawHeading(const char* title);
	};

}

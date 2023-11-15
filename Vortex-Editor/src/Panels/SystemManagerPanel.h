#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class SystemManagerPanel : public EditorPanel
	{
	public:
		SystemManagerPanel() = default;
		~SystemManagerPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(SystemManager)
	};

}

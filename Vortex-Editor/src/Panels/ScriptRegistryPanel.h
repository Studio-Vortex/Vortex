#pragma once

#include <Vortex.h>

#include  <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class ScriptRegistryPanel : public EditorPanel
	{
	public:
		~ScriptRegistryPanel() override = default;

		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(ScriptRegistry)
	};

}

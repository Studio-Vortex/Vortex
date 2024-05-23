#pragma once

#include <Vortex.h>

#include <Vortex/Editor/EditorPanel.h>

namespace Vortex {

	class InputManagerPanel : public EditorPanel
	{
	public:
		InputManagerPanel() = default;
		~InputManagerPanel() override = default;

		void OnPanelAttach() override;
		void OnGuiRender() override;

		EDITOR_PANEL_TYPE(InputManager)

	private:
		void RenderAddKeybindPopup();
		void RenderKeybinds();

		void RenderAddMousebindPopup();
		void RenderMousebinds();

	private:
		std::vector<const char*> m_KeycodeOptions;
		std::vector<const char*> m_MousebuttonOptions;

		bool m_AddKeybindPopupOpen = false;
		bool m_AddMousebindPopupOpen = false;
	};

}

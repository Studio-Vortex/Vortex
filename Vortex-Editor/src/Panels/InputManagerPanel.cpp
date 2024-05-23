#include "InputManagerPanel.h"

namespace Vortex {

	void InputManagerPanel::OnGuiRender()
	{
		if (!IsOpen)
			return;

		Gui::Begin(m_PanelName.c_str(), &IsOpen);

		if (Gui::Button("Add Keybind"))
		{
			m_AddKeybindPopupOpen = true;
		}

		RenderKeybinds();

		if (Gui::Button("Add Mousebind"))
		{
			m_AddMousebindPopupOpen = true;
		}

		RenderMousebinds();

		RenderAddKeybindPopup();
		RenderAddMousebindPopup();

		Gui::End();
	}

	void InputManagerPanel::RenderAddKeybindPopup()
	{
		const std::string popupName = "Add Keybind";
		if (m_AddKeybindPopupOpen)
		{
			Gui::OpenPopup(popupName.c_str());
		}

		if (UI::ShowMessageBox(popupName.c_str(), &m_AddKeybindPopupOpen, { 500, 220 }))
		{
			UI::Draw::Underline();
			Gui::Spacing();

			ImVec2 buttonSize(Gui::GetFontSize() * 13.27f, 0.0f);

			Gui::TextCentered("Enter keybind name and choose a key to bind to", 40.0f);

			Gui::Spacing();
			UI::Draw::Underline();

			UI::BeginPropertyGrid();

			static std::string keybindName = "(null)";
			UI::Property("Keybind Name", keybindName);

			static std::vector<const char*> keycodeOptions;
			if (keycodeOptions.empty())
			{
				for (uint32_t i = (uint32_t)KeyCode::Space; i < (uint32_t)KeyCode::MaxKeys; i++)
				{
					const char* keycodeStr = Utils::StringFromKeyCode((KeyCode)i);
					if (std::string(keycodeStr).starts_with("Unknown keycode!"))
						continue;
					keycodeOptions.push_back(keycodeStr);
				}
			}
			KeyCode currentKeycode = KeyCode::Space;
			UI::KeyCodeDropdown("KeyCode", keycodeOptions.data(), keycodeOptions.size(), currentKeycode);

			UI::EndPropertyGrid();

			UI::ShiftCursorY(20.0f);
			UI::ShiftCursorX(7.0f);

			auto resetPopup = [&]() {
				keybindName = "(null)";
				m_AddKeybindPopupOpen = false;
			};

			if (Gui::Button("Add", buttonSize))
			{
				

				resetPopup();
				Gui::CloseCurrentPopup();
			}

			Gui::SameLine();

			if (Gui::Button("Cancel", buttonSize))
			{
				resetPopup();
				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}
	}

	void InputManagerPanel::RenderKeybinds()
	{
		const InputManager::KeybindMap& keybinds = InputManager::GetAllKeybinds();

		for (const auto& [name, keycode] : keybinds)
		{
			
		}
	}

	void InputManagerPanel::RenderAddMousebindPopup()
	{
	}

	void InputManagerPanel::RenderMousebinds()
	{
		const InputManager::MousebindMap& mousebinds = InputManager::GetAllMousebinds();

		for (const auto& [name, mouseButton] : mousebinds)
		{

		}
	}

}

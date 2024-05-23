#include "InputManagerPanel.h"

namespace Vortex {

	void InputManagerPanel::OnPanelAttach()
	{
		for (uint32_t i = (uint32_t)KeyCode::Space; i < (uint32_t)KeyCode::MaxKeys; i++)
		{
			const char* keycodeStr = Utils::StringFromKeyCode((KeyCode)i);
			if (std::string(keycodeStr).starts_with("Unknown keycode!"))
				continue;
			m_KeycodeOptions.push_back(keycodeStr);
		}

		for (uint32_t i = (uint32_t)MouseButton::Button0; i < (uint32_t)MouseButton::MaxButtons; i++)
		{
			const char* mousebuttonStr = Utils::StringFromMouseButton((MouseButton)i);
			if (std::string(mousebuttonStr).starts_with("Unknown mouse button!"))
				continue;
			m_MousebuttonOptions.push_back(mousebuttonStr);
		}
	}

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

			static KeyCode currentKeycode = KeyCode::Space;
			UI::KeyCodeDropdown("KeyCode", m_KeycodeOptions.data(), m_KeycodeOptions.size(), currentKeycode);

			UI::EndPropertyGrid();

			UI::ShiftCursorY(20.0f);
			UI::ShiftCursorX(7.0f);

			auto resetPopup = [&]() {
				keybindName = "(null)";
				currentKeycode = KeyCode::Space;
				m_AddKeybindPopupOpen = false;
			};

			if (Gui::Button("Add", buttonSize))
			{
				InputManager::SetKeybindByString(keybindName, currentKeycode);

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

		static const char* columns[] = { "Name", "KeyCode" };

		ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		ImVec2 tableSize = { contentRegionAvail.x, 275.0f };
		UI::Table("Keybinds", columns, VX_ARRAYSIZE(columns), tableSize, [&]()
		{
			for (const auto& [name, keycode] : keybinds)
			{
				Gui::TableNextColumn();
				UI::ShiftCursorX(10.0f);
				Gui::Text(name.c_str());
				UI::Draw::Underline();

				Gui::TableNextColumn();
				const std::string dropdownName = name;

				UI::ShiftCursorY(-12.0f);

				KeyCode currentKeycode = keycode;
				if (UI::KeyCodeDropdown(dropdownName.c_str(), m_KeycodeOptions.data(), m_KeycodeOptions.size(), currentKeycode, false))
				{
					InputManager::SetKeybindByString(name, currentKeycode);
				}
			}
		});
	}

	void InputManagerPanel::RenderAddMousebindPopup()
	{
		const std::string popupName = "Add Mousebind";
		if (m_AddMousebindPopupOpen)
		{
			Gui::OpenPopup(popupName.c_str());
		}

		if (UI::ShowMessageBox(popupName.c_str(), &m_AddMousebindPopupOpen, { 500, 220 }))
		{
			UI::Draw::Underline();
			Gui::Spacing();

			ImVec2 buttonSize(Gui::GetFontSize() * 13.27f, 0.0f);

			Gui::TextCentered("Enter mousebind name and choose a button to bind to", 40.0f);

			Gui::Spacing();
			UI::Draw::Underline();

			UI::BeginPropertyGrid();

			static std::string mousebindName = "(null)";
			UI::Property("Mousebind Name", mousebindName);

			static MouseButton currentMousebind = MouseButton::Left;
			UI::MouseButtonDropdown("Mouse Button", m_MousebuttonOptions.data(), m_MousebuttonOptions.size(), currentMousebind);

			UI::EndPropertyGrid();

			UI::ShiftCursorY(20.0f);
			UI::ShiftCursorX(7.0f);

			auto resetPopup = [&]() {
				mousebindName = "(null)";
				currentMousebind = MouseButton::Left;
				m_AddMousebindPopupOpen = false;
			};

			if (Gui::Button("Add", buttonSize))
			{
				InputManager::SetMousebindByString(mousebindName, currentMousebind);

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

	void InputManagerPanel::RenderMousebinds()
	{
		const InputManager::MousebindMap& mousebinds = InputManager::GetAllMousebinds();

		static const char* columns[] = { "Name", "Mouse Button" };

		ImVec2 contentRegionAvail = Gui::GetContentRegionAvail();
		ImVec2 tableSize = { contentRegionAvail.x, 275.0f };
		UI::Table("Mousebinds", columns, VX_ARRAYSIZE(columns), tableSize, [&]()
		{
			for (const auto& [name, mousebutton] : mousebinds)
			{
				Gui::TableNextColumn();
				UI::ShiftCursorX(10.0f);
				Gui::Text(name.c_str());
				UI::Draw::Underline();

				Gui::TableNextColumn();
				const std::string dropdownName = name;

				UI::ShiftCursorY(-12.0f);

				MouseButton currentMousebutton = mousebutton;
				if (UI::MouseButtonDropdown(dropdownName.c_str(), m_MousebuttonOptions.data(), m_MousebuttonOptions.size(), currentMousebutton, false))
				{
					InputManager::SetMousebindByString(name, currentMousebutton);
				}
			}
		});
	}

}

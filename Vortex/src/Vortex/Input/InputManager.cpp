#include "vxpch.h"
#include "InputManager.h"

namespace Vortex {

	void InputManager::SetKeybindByString(const std::string& name, KeyCode key)
	{
		s_Keybinds[name] = key;
	}

	KeyCode InputManager::GetKeybindByString(const std::string& name)
	{
		if (HasKeybind(name))
			return s_Keybinds[name];

		return KeyCode::A;
	}

	bool InputManager::HasKeybind(const std::string& name)
	{
		return s_Keybinds.contains(name);
	}

	void InputManager::SetMousebindByString(const std::string& name, MouseButton button)
	{
		s_Mousebinds[name] = button;
	}

	MouseButton InputManager::GetMousebindByString(const std::string& name)
	{
		if (HasMousebind(name))
			return s_Mousebinds[name];

		return MouseButton::Button0;
	}

	bool InputManager::HasMousebind(const std::string& name)
	{
		return s_Mousebinds.contains(name);
	}

	const InputManager::KeybindMap& InputManager::GetAllKeybinds()
	{
		return s_Keybinds;
	}

	const InputManager::MousebindMap& InputManager::GetAllMousebinds()
	{
		return s_Mousebinds;
	}

}

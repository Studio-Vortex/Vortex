#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Input/KeyCodes.h"
#include "Vortex/Input/MouseCodes.h"

#include <unordered_map>
#include <string>

namespace Vortex {
	
	class VORTEX_API InputManager
	{
	public:
		using VORTEX_API KeybindMap = std::unordered_map<std::string, KeyCode>;
		using VORTEX_API MousebindMap = std::unordered_map<std::string, MouseButton>;

	public:
		static void SetKeybindByString(const std::string& name, KeyCode key);
		static KeyCode GetKeybindByString(const std::string& name);
		static bool HasKeybind(const std::string& name);

		static void SetMousebindByString(const std::string& name, MouseButton button);
		static MouseButton GetMousebindByString(const std::string& name);
		static bool HasMousebind(const std::string& name);

		static const KeybindMap& GetAllKeybinds();
		static const MousebindMap& GetAllMousebinds();

	private:
		static inline KeybindMap s_Keybinds;
		static inline MousebindMap s_Mousebinds;
	};

}
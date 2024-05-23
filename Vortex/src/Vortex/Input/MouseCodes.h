#pragma once

#include <iostream>

namespace Vortex {

#define VX_MAX_MOUSE_BUTTONS 7

	enum class MouseButton : uint16_t
	{
		// From glfw3.h
		Button0 = 0,
		Button1 = 1,
		Button2 = 2,
		Button3 = 3,
		Button4 = 4,
		Button5 = 5,
		Button6 = 6,
		Button7 = 7,

		Last = Button7,
		Left = Button0,
		Right = Button1,
		Middle = Button2,

		MaxButtons = Button7,
	};

	enum class CursorMode : uint16_t
	{
		// From glfw3.h
		Normal = 0,
		Hidden = 1,
		Locked = 2,
	};

	namespace Utils {

		const char* StringFromMouseButton(MouseButton button);
		MouseButton MouseButtonFromString(const char* str);

	}

	inline std::ostream& operator<<(std::ostream& stream, MouseButton mouseCode)
	{
		return stream << static_cast<int32_t>(mouseCode);
	}

	inline std::ostream& operator<<(std::ostream& stream, CursorMode cursorMode)
	{
		return stream << static_cast<uint32_t>(cursorMode);
	}

}

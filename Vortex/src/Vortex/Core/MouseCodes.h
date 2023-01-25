#pragma once

#include <iostream>

namespace Vortex {

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
	};

	enum class CursorMode : uint16_t
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2,
	};

	inline std::ostream& operator<<(std::ostream& os, MouseButton mouseCode)
	{
		os << static_cast<int32_t>(mouseCode);
		return os;
	}

	inline std::ostream& operator<<(std::ostream& os, CursorMode cursorMode)
	{
		os << static_cast<uint32_t>(cursorMode);
		return os;
	}

}

#pragma once

namespace Sparky
{
	typedef enum class MouseCode : uint16_t
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

		ButtonLast = Button7,
		ButtonLeft = Button0,
		ButtonRight = Button1,
		ButtonMiddle = Button2
	} Mouse;

	inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
	{
		os << static_cast<int32_t>(mouseCode);
		return os;
	}
}

#define SP_MOUSE_BUTTON_0      ::Sparky::Mouse::Button0
#define SP_MOUSE_BUTTON_1      ::Sparky::Mouse::Button1
#define SP_MOUSE_BUTTON_2      ::Sparky::Mouse::Button2
#define SP_MOUSE_BUTTON_3      ::Sparky::Mouse::Button3
#define SP_MOUSE_BUTTON_4      ::Sparky::Mouse::Button4
#define SP_MOUSE_BUTTON_5      ::Sparky::Mouse::Button5
#define SP_MOUSE_BUTTON_6      ::Sparky::Mouse::Button6
#define SP_MOUSE_BUTTON_7      ::Sparky::Mouse::Button7
#define SP_MOUSE_BUTTON_LAST   ::Sparky::Mouse::ButtonLast
#define SP_MOUSE_BUTTON_LEFT   ::Sparky::Mouse::ButtonLeft
#define SP_MOUSE_BUTTON_RIGHT  ::Sparky::Mouse::ButtonRight
#define SP_MOUSE_BUTTON_MIDDLE ::Sparky::Mouse::ButtonMiddle
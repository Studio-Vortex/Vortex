#pragma once

#include <iostream>

namespace Sparky {

	typedef enum class Gamepad : uint16_t
	{
		// From glfw3.h
		JoyStick1                   = 0,
		JoyStick2                   = 1,
		JoyStick3                   = 2,
		JoyStick4                   = 3,
		JoyStick5                   = 4,
		JoyStick6                   = 5,
		JoyStick7                   = 6,
		JoyStick8                   = 7,
		JoyStick9                   = 8,
		JoyStick10                  = 9,
		JoyStick11                  = 10,
		JoyStick12                  = 11,
		JoyStick13                  = 12,
		JoyStick14                  = 13,
		JoyStick15                  = 14,
		JoyStick16                  = 15,
		JoyStickLast                = JoyStick16,

		ButtonA                     = 0,
		ButtonB                     = 1,
		ButtonX                     = 2,
		ButtonY                     = 3,
		LeftBumper                  = 4,
		RightBumper                 = 5,
		Back                        = 6,
		Start                       = 7,
		Guide                       = 8,
		LeftThumb                   = 9,
		RightThumb                  = 10,
		DpadUp                      = 11,
		DpadRight                   = 12,
		DpadDown                    = 13,
		DpadLeft                    = 14,
		ButtonLast                  = DpadLeft,

		Cross                       = ButtonA,
		Circle                      = ButtonB,
		Square                      = ButtonX,
		Triangle                    = ButtonY,
 
		AxisLeftX                   = 0,
		AxisLeftY                   = 1,
		AxisRightX                  = 2,
		AxisRightY                  = 3,
		AxisLeftTrigger             = 4,
		AxisRightTrigger            = 5,
		AxisLast                    = AxisRightTrigger,
	} Gamepad;

	inline std::ostream& operator<<(std::ostream& os, Gamepad gamepad)
	{
		os << static_cast<int32_t>(gamepad);
		return os;
	}

}

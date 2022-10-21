#pragma once

#include <iostream>

namespace Sparky {

	typedef enum class Gamepad : uint16_t
	{
		// From glfw3.h
		ButtonA                     = 0,
		ButtonB                     = 1,
		ButtonX                     = 2,
		ButtonY                     = 3,
		LeftBumper                  = 4,
		RightBumper                 = 5,
		ButtonGuide                 = 6,
		ButtonStart                 = 7,
		ButtonBack                  = 8,
		LeftStick                   = 9,
		RightStick                  = 10,
		Up                          = 11,
		Right                       = 12,
		Down                        = 13,
		Left                        = 14,
		ButtonLast                  = Left,

		ButtonCross                 = ButtonA,
		ButtonCircle                = ButtonB,
		ButtonSquare                = ButtonX,
		ButtonTriangle              = ButtonY,
 
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

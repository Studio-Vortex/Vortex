#pragma once

#include <iostream>

namespace Vortex {

	enum class GamepadButton : uint16_t
	{
		// From glfw3.h
		A           = 0,
		B           = 1,
		X           = 2,
		Y           = 3,

		LeftBumper  = 4,
		RightBumper = 5,
		Guide       = 6,
		Start       = 7,
		Back        = 8,
		LeftStick   = 9,
		RightStick  = 10,

		Up          = 11,
		Right       = 12,
		Down        = 13,
		Left        = 14,
				    
		Cross       = A,
		Circle      = B,
		Square      = X,
		Triangle    = Y,

		Last        = Left,
	};

	enum class GamepadAxis : uint16_t
	{
		// From glfw3.h
		LeftX        = 0,
		LeftY        = 1,
		RightX       = 2,
		RightY       = 3,
		LeftTrigger  = 4,
		RightTrigger = 5,
		Last         = RightTrigger,
	};

	inline std::ostream& operator<<(std::ostream& stream, GamepadButton gamepadButton)
	{
		return stream << static_cast<int32_t>(gamepadButton);
	}

	inline std::ostream& operator<<(std::ostream& stream, GamepadAxis gamepadAxis)
	{
		return stream << static_cast<int32_t>(gamepadAxis);
	}

}

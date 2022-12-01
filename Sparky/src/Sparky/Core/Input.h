#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Core/KeyCodes.h"
#include "Sparky/Core/MouseCodes.h"
#include "Sparky/Core/Gamepad.h"
#include "Sparky/Core/Math.h"

namespace Sparky {

	class SPARKY_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);
		static bool IsKeyReleased(KeyCode keycode);

		static bool IsMouseButtonPressed(MouseCode button);
		static bool IsMouseButtonReleased(MouseCode button);

		static bool IsGamepadButtonPressed(Gamepad button);
		static bool IsGamepadButtonReleased(Gamepad button);

		static float GetGamepadAxis(Gamepad axis);

		static Math::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};

}
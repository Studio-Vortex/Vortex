#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/KeyCodes.h"
#include "Vortex/Core/MouseCodes.h"
#include "Vortex/Core/Gamepad.h"
#include "Vortex/Core/Math.h"
#include "Vortex/Events/Event.h"

namespace Vortex {

	class VORTEX_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);
		static bool IsKeyReleased(KeyCode keycode);

		static bool IsMouseButtonPressed(MouseButton button);
		static bool IsMouseButtonReleased(MouseButton button);

		static bool IsGamepadButtonPressed(Gamepad button);
		static bool IsGamepadButtonReleased(Gamepad button);

		static Math::vec2 GetMouseScrollOffset();
		static void SetMouseScrollOffset(const Math::vec2& offset);

		static float GetGamepadAxis(Gamepad axis);

		static Math::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void SetCursorPosition(const Math::vec2& position);

		static CursorMode GetCursorMode();
		static void SetCursorMode(CursorMode cursorMode);

		// For Internal Use
		static void UpdateMouseState(const Event& event);
	};

}
#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Input/KeyCodes.h"
#include "Vortex/Core/Input/MouseCodes.h"
#include "Vortex/Core/Input/Gamepad.h"
#include "Vortex/Core/Math/Math.h"
#include "Vortex/Events/Event.h"

namespace Vortex {

	class VORTEX_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode);
		static bool IsKeyReleased(KeyCode keycode);
		static bool IsKeyDown(KeyCode keycode);
		static bool IsKeyUp(KeyCode keycode);

		static bool IsMouseButtonPressed(MouseButton button);
		static bool IsMouseButtonReleased(MouseButton button);
		static bool IsMouseButtonDown(MouseButton button);
		static bool IsMouseButtonUp(MouseButton button);

		static bool IsGamepadButtonDown(GamepadButton button);
		static bool IsGamepadButtonUp(GamepadButton button);

		static Math::vec2 GetMouseScrollOffset();
		static void SetMouseScrollOffset(const Math::vec2& offset);

		static float GetGamepadAxis(GamepadAxis axis);

		static Math::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();

		static void SetMousePosition(const Math::vec2& position);

		static CursorMode GetCursorMode();
		static void SetCursorMode(CursorMode cursorMode);

		// For Internal Use
		static void OnEvent(const Event& event);
		static void UpdateKeyState(KeyCode key, int action);
		static void UpdateMouseButtonState(MouseButton button, int action);
		static bool KeyChangedThisFrame(KeyCode key);
		static bool MouseButtonChangedThisFrame(MouseButton mousebutton);
		static void ResetChangesForNextFrame();
	};

}
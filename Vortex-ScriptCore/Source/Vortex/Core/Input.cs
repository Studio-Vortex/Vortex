namespace Vortex {

	public static class Input
	{
		public static bool IsKeyPressed(KeyCode key) => InternalCalls.Input_IsKeyPressed(key);
		public static bool IsKeyReleased(KeyCode key) => InternalCalls.Input_IsKeyReleased(key);
		public static bool IsKeyDown(KeyCode key) => InternalCalls.Input_IsKeyDown(key);
		public static bool IsKeyUp(KeyCode key) => InternalCalls.Input_IsKeyUp(key);

		public static bool IsMouseButtonPressed(MouseButton mouseButton) => InternalCalls.Input_IsMouseButtonPressed(mouseButton);
		public static bool IsMouseButtonReleased(MouseButton mouseButton) => InternalCalls.Input_IsMouseButtonReleased(mouseButton);
		public static bool IsMouseButtonDown(MouseButton mouseButton) => InternalCalls.Input_IsMouseButtonDown(mouseButton);
		public static bool IsMouseButtonUp(MouseButton mouseButton) => InternalCalls.Input_IsMouseButtonUp(mouseButton);

		public static CursorMode GetCursorMode() => InternalCalls.Input_GetCursorMode();
		public static void SetCursorMode(CursorMode mode) => InternalCalls.Input_SetCursorMode(mode);

		public static Vector2 MousePosition
		{
			get
			{
				InternalCalls.Input_GetMousePosition(out Vector2 result);
				return result;
			}

			set => InternalCalls.Input_SetMousePosition(ref value);
		}

		public static Vector2 GetMouseScrollDelta()
		{
			InternalCalls.Input_GetMouseScrollOffset(out Vector2 result);
			return result;
		}

		public static bool IsGamepadButtonDown(Gamepad button) => InternalCalls.Input_IsGamepadButtonDown(button);
		public static bool IsGamepadButtonUp(Gamepad button) => InternalCalls.Input_IsGamepadButtonUp(button);
		public static float GetGamepadAxis(Gamepad axis) => InternalCalls.Input_GetGamepadAxis(axis);
	}

}

namespace Sparky {

	public static class Input
	{
		public static bool ShowMouseCursor
		{
			set
			{
				InternalCalls.Input_ShowMouseCursor(value);
			}
		}

		public static bool IsKeyDown(KeyCode key)
		{
			return InternalCalls.Input_IsKeyDown(key);
		}

		public static bool IsKeyUp(KeyCode key)
		{
			return InternalCalls.Input_IsKeyUp(key);
		}

		public static bool IsMouseButtonDown(MouseButton mouseButton)
		{
			return InternalCalls.Input_IsMouseButtonDown(mouseButton);
		}

		public static bool IsMouseButtonUp(MouseButton mouseButton)
		{
			return InternalCalls.Input_IsMouseButtonUp(mouseButton);
		}

		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 position);
			return position;
		}

		public static bool IsGamepadButtonDown(Gamepad button)
		{
			return InternalCalls.Input_IsGamepadButtonDown(button);
		}

		public static bool IsGamepadButtonUp(Gamepad button)
		{
			return InternalCalls.Input_IsGamepadButtonUp(button);
		}

		public static float GetGamepadAxis(Gamepad axis)
		{
			return InternalCalls.Input_GetGamepadAxis(axis);
		}
	}

}

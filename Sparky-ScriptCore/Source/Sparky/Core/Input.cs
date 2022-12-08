using System.Runtime.InteropServices.WindowsRuntime;

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

		public static bool IsKeyDown(KeyCode key) => InternalCalls.Input_IsKeyDown(key);

		public static bool IsKeyUp(KeyCode key) => InternalCalls.Input_IsKeyUp(key);

		public static bool IsMouseButtonDown(MouseButton mouseButton) => InternalCalls.Input_IsMouseButtonDown(mouseButton);

		public static bool IsMouseButtonUp(MouseButton mouseButton) => InternalCalls.Input_IsMouseButtonUp(mouseButton);

		public static Vector2 GetMousePosition()
		{
			InternalCalls.Input_GetMousePosition(out Vector2 position);
			return position;
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

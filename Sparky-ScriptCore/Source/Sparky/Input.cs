namespace Sparky {

	public static class Input
	{
		public static bool IsKeyDown(KeyCode key)
		{
			return InternalCalls.Input_IsKeyDown(key);
		}

		public static bool IsKeyUp(KeyCode key)
		{
			return InternalCalls.Input_IsKeyUp(key);
		}
	}

}

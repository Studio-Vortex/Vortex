namespace Sparky {

	public static class Window
	{
		public static Vector2 GetSize()
		{
			InternalCalls.Window_GetSize(out Vector2 size);
			return size;
		}

		public static Vector2 GetPosition()
		{
			InternalCalls.Window_GetPosition(out Vector2 position);
			return position;
		}

		public static void ShowMouseCursor(bool enabled)
		{
			InternalCalls.Window_ShowMouseCursor(enabled);
		}
	}

}

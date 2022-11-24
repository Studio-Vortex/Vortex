namespace Sparky {

	public static class Application
	{
		public static void Quit()
		{
			InternalCalls.Application_Quit();
		}

		public static Vector2 GetSize()
		{
			InternalCalls.Application_GetSize(out Vector2 size);
			return size;
		}

		public static Vector2 GetPosition()
		{
			InternalCalls.Application_GetPosition(out Vector2 position);
			return position;
		}

		public static bool IsMaximized()
		{
			return InternalCalls.Application_IsMaximized();
		}
	}

}

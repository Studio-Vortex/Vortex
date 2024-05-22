namespace Vortex {

	public static class Window
	{
		public static Vector2 Size
		{
			get
			{
				InternalCalls.Window_GetSize(out Vector2 size);
				return size;
			}
		}

		public static Vector2 Position
		{
			get
			{
				InternalCalls.Window_GetPosition(out Vector2 position);
				return position;
			}
		}

		public static bool Maximized
		{
			get => InternalCalls.Window_IsMaximized();
			set => InternalCalls.Window_SetMaximized(value);
		}

		public static bool Resizeable
		{
			get => InternalCalls.Window_IsResizeable();
			set => InternalCalls.Window_SetResizeable(value);
		}

		public static bool Decorated
		{
			get => InternalCalls.Window_IsDecorated();
			set => InternalCalls.Window_SetDecorated(value);
		}

		public static bool IsVSyncEnabled() => InternalCalls.Window_IsVSyncEnabled();
		public static void SetVSync(bool use) => InternalCalls.Window_SetVSync(use);
	}

}

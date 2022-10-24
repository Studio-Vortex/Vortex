namespace Sparky {

	public static class Gui
	{
		public static void Begin(string text)
		{
			InternalCalls.Gui_Begin(text);
		}

		public static void Begin(string text, Vector2 position)
		{
			InternalCalls.Gui_BeginWithPosition(text, ref position);
		}
		
		public static void Begin(string text, float width, float height)
		{
			InternalCalls.Gui_BeginWithSize(text, width, height);
		}
		
		public static void Begin(string text, Vector2 position, Vector2 size)
		{
			InternalCalls.Gui_BeginWithPositionAndSize(text, ref position, ref size);
		}

		public static void End()
		{
			InternalCalls.Gui_End();
		}

		public static void Separator()
		{
			InternalCalls.Gui_Separator();
		}

		public static void Spacing()
		{
			InternalCalls.Gui_Spacing();
		}

		public static void Text(string text)
		{
			InternalCalls.Gui_Text(text);
		}

		public static bool Button(string text)
		{
			return InternalCalls.Gui_Button(text);
		}
	}

}

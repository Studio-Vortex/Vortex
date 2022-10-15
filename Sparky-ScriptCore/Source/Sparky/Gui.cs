namespace Sparky {

	public static class Gui
	{
		public static void Begin(string text)
		{
			InternalCalls.Gui_Begin(text);
		}

		public static void End()
		{
			InternalCalls.Gui_End();
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

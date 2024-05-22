namespace Vortex {

	public static class Gui
	{
		public static void Begin(string label) => InternalCalls.Gui_Begin(label);
		public static void End() => InternalCalls.Gui_End();

		public static void Underline() => InternalCalls.Gui_Underline();
		public static void Spacing(ulong count = 1) => InternalCalls.Gui_Spacing(count);

		public static void Text(string text) => InternalCalls.Gui_Text(text);
		public static bool Button(string label) => InternalCalls.Gui_Button(label);
		public static bool Button(string label, Vector2 size) => InternalCalls.Gui_ButtonWithSize(label, ref size);

		public static void BeginPropertyGrid() => InternalCalls.Gui_BeginPropertyGrid();
		public static void EndPropertyGrid() => InternalCalls.Gui_EndPropertyGrid();

		public static bool PropertyGridHeader(string label, bool defaultOpen = true) => InternalCalls.Gui_PropertyGridHeader(label, defaultOpen);
		public static void EndGridHeader() => InternalCalls.Gui_EndGridHeader();

		public static bool Property(string label, ref bool value) => InternalCalls.Gui_PropertyBool(label, ref value);
		public static bool Property(string label, ref int value) => InternalCalls.Gui_PropertyInt(label, ref value);
		public static bool Property(string label, ref ulong value) => InternalCalls.Gui_PropertyULong(label, ref value);
		public static bool Property(string label, ref float value) => InternalCalls.Gui_PropertyFloat(label, ref value);
		public static bool Property(string label, ref double value) => InternalCalls.Gui_PropertyDouble(label, ref value);
		public static bool Property(string label, ref Vector2 value) => InternalCalls.Gui_PropertyVec2(label, ref value);
		public static bool Property(string label, ref Vector3 value) => InternalCalls.Gui_PropertyVec3(label, ref value);
		public static bool Property(string label, ref Vector4 value) => InternalCalls.Gui_PropertyVec4(label, ref value);
		public static bool Property(string label, ref Color3 value) => InternalCalls.Gui_PropertyColor3(label, ref value);
		public static bool Property(string label, ref Color4 value) => InternalCalls.Gui_PropertyColor4(label, ref value);
	}

}

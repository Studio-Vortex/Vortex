namespace Vortex {

	public static class Gui
	{
		public static void Begin(string text) => InternalCalls.Gui_Begin(text);
		public static void Begin(string text, Vector2 position) => InternalCalls.Gui_BeginWithPosition(text, ref position);
		public static void Begin(string text, float width, float height) => InternalCalls.Gui_BeginWithSize(text, width, height);
		public static void Begin(string text, Vector2 position, Vector2 size) => InternalCalls.Gui_BeginWithPositionAndSize(text, ref position, ref size);
		public static void End() => InternalCalls.Gui_End();

		public static void Underline() => InternalCalls.Gui_Underline();
		public static void Spacing(ulong count = 1) => InternalCalls.Gui_Spacing(count);

		public static void Text(string text) => InternalCalls.Gui_Text(text);
		public static bool Button(string text) => InternalCalls.Gui_Button(text);

		public static bool Property(string label, out bool value) => InternalCalls.Gui_PropertyBool(label, out value);
		public static bool Property(string label, out int value) => InternalCalls.Gui_PropertyInt(label, out value);
		public static bool Property(string label, out ulong value) => InternalCalls.Gui_PropertyULong(label, out value);
		public static bool Property(string label, out float value) => InternalCalls.Gui_PropertyFloat(label, out value);
		public static bool Property(string label, out double value) => InternalCalls.Gui_PropertyDouble(label, out value);
		public static bool Property(string label, out Vector2 value) => InternalCalls.Gui_PropertyVec2(label, out value);
		public static bool Property(string label, out Vector3 value) => InternalCalls.Gui_PropertyVec3(label, out value);
		public static bool Property(string label, out Vector4 value) => InternalCalls.Gui_PropertyVec4(label, out value);
		public static bool Property(string label, out Color3 value) => InternalCalls.Gui_PropertyColor3(label, out value);
		public static bool Property(string label, out Color4 value) => InternalCalls.Gui_PropertyColor4(label, out value);
	}

}

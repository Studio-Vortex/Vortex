namespace Vortex.Renderer {

	public static class DebugRenderer
	{
		public static float LineWidth
		{
			get => InternalCalls.DebugRenderer_GetLineWidth();
			set => InternalCalls.DebugRenderer_SetLineWidth(value);
		}

		public static void DrawLine(Vector3 startPoint, Vector3 endPoint, Vector4 color)
		{
			InternalCalls.DebugRenderer_DrawLine(ref startPoint, ref endPoint, ref color);
		}

		public static void DrawQuadBillboard(Vector3 translation, Vector2 size, Vector4 color)
		{
			InternalCalls.DebugRenderer_DrawQuadBillboard(ref translation, ref size, ref color);
		}

		public static void DrawCircle(Vector2 translation, Vector2 size, Vector4 color, float thickness = 1.0f, float fade = 0.005f)
		{
			InternalCalls.DebugRenderer_DrawCircleVec2(ref translation, ref size, ref color, thickness, fade);
		}

		public static void DrawCircle(Vector3 translation, Vector3 size, Vector4 color, float thickness = 1.0f, float fade = 0.005f)
		{
			InternalCalls.DebugRenderer_DrawCircleVec3(ref translation, ref size, ref color, thickness, fade);
		}

		public static void DrawBoundingBox(Vector3 translation, Vector3 size, Vector4 color)
		{
			InternalCalls.DebugRenderer_DrawBoundingBox(ref translation, ref size, ref color);
		}

		public static void DrawBoundingBox(Actor actor, Vector4 color)
		{
			InternalCalls.DebugRenderer_DrawBoundingBoxFromTransform(actor.ID, ref color);
		}

		public static void Flush() => InternalCalls.DebugRenderer_Flush();
	}

}

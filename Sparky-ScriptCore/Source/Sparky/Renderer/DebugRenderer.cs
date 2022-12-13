﻿namespace Sparky {

	public static class DebugRenderer
	{
		public static void BeginScene()
		{
			InternalCalls.DebugRenderer_BeginScene();
		}

		public static void SetClearColor(Vector3 color)
		{
			InternalCalls.DebugRenderer_SetClearColor(ref color);
		}

		public static void DrawLine(Vector3 p1, Vector3 p2, Vector4 color)
		{
			InternalCalls.DebugRenderer_DrawLine(ref p1, ref p2, ref color);
		}

		public static void DrawQuadBillboard(Vector3 translation, Vector2 size, Vector4 color)
		{
			InternalCalls.DebugRenderer_DrawQuadBillboard(ref translation, ref size, ref color);
		}

		public static void DrawBoundingBox(Vector3 translation, Vector3 size, Vector4 color)
		{
			InternalCalls.DebugRenderer_DrawBoundingBox(ref translation, ref size, ref color);
		}

		public static void Flush()
		{
			InternalCalls.DebugRenderer_Flush();
		}
	}

}

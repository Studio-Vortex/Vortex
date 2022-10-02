namespace Sparky {

	public static class Renderer
	{
		public static void SetClearColor(Vector3 color)
		{
			InternalCalls.Renderer_SetClearColor(ref color);
		}
	}

}

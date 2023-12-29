namespace Vortex {

	public static class SceneRenderer
	{
		public static float Exposure
		{
			get => InternalCalls.SceneRenderer_GetExposure();
			set => InternalCalls.SceneRenderer_SetExposure(value);
		}

		public static float Gamma
		{
			get => InternalCalls.SceneRenderer_GetGamma();
			set => InternalCalls.SceneRenderer_SetGamma(value);
		}
	}

}

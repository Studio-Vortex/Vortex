namespace Vortex {

	public struct BloomSettings
	{
		public float Threshold
		{
			get => InternalCalls.SceneRenderer_GetBloomThreshold();
			set => InternalCalls.SceneRenderer_SetBloomThreshold(value);
		}

		public float SoftKnee
		{
			get => InternalCalls.SceneRenderer_GetBloomSoftKnee();
			set => InternalCalls.SceneRenderer_SetBloomSoftKnee(value);
		}

		public float Unknown
		{
			get => InternalCalls.SceneRenderer_GetBloomUnknown();
			set => InternalCalls.SceneRenderer_SetBloomUnknown(value);
		}
	}

	public static class SceneRenderer
	{
		public static BloomSettings Bloom;

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

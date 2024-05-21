namespace Vortex
{
	public class FogProperties
	{
		public bool Enabled
		{
			get => InternalCalls.SceneRenderer_GetFogEnabled();
			set => InternalCalls.SceneRenderer_SetFogEnabled(value);
		}

		public float Density
		{
			get => InternalCalls.SceneRenderer_GetFogDensity();
			set => InternalCalls.SceneRenderer_SetFogDensity(value);
		}

		public float Gradient
		{
			get => InternalCalls.SceneRenderer_GetFogGradient();
			set => InternalCalls.SceneRenderer_SetFogGradient(value);
		}
	}
}

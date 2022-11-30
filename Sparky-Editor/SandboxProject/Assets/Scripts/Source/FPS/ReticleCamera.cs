using Sparky;

namespace Sandbox {

	public class ReticleCamera : Entity
	{
		public float reticleSize;
		public Vector4 reticleColor;

		protected override void OnUpdate(float deltaTime)
		{
			DrawReticle();
		}

		void DrawReticle()
		{
			DebugRenderer.BeginScene();
			DebugRenderer.DrawQuadBillboard(transform.Translation + transform.Forward, new Vector2(0.0075f, reticleSize), reticleColor);
			DebugRenderer.Flush();
		}
	}

}

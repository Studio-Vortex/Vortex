using Sparky;

namespace Sandbox {

	public class UI_Reticle : Entity
	{
		public Vector4 color;
		public Vector2 size;

		protected override void OnUpdate(float deltaTime)
		{
			DebugRenderer.BeginScene();
			DebugRenderer.DrawQuadBillboard(transform.Translation, size, color);
			DebugRenderer.Flush();
		}
	}

}

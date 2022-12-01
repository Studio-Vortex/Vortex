using Sparky;

namespace Sandbox {

	public class UI_Reticle : Entity
	{
		public Vector4 color;
		public Vector2 size;

		protected override void OnUpdate(float deltaTime)
		{
			// If the player zooms in don't show the reticle
			if (!Input.IsMouseButtonDown(MouseButton.Right))
			{
				DebugRenderer.BeginScene();
				DebugRenderer.DrawQuadBillboard(transform.Translation, size, color);
				DebugRenderer.Flush();
			}
		}
	}

}

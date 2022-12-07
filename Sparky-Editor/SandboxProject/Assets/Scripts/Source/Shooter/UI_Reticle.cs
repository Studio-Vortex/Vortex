using Sparky;

namespace Sandbox {

	public class UI_Reticle : Entity
	{
		public Vector4 color;
		public Vector2 size;

		protected override void OnUpdate(float deltaTime)
		{
			// If the player zooms in don't show the reticle
			bool rightMouseButtonPressed = Input.IsMouseButtonDown(MouseButton.Right);
			bool leftTriggerPressed = Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > 0f;

			if (!rightMouseButtonPressed && !leftTriggerPressed)
			{
				DebugRenderer.BeginScene();
				DebugRenderer.DrawQuadBillboard(transform.Translation + new Vector3(0.01f, 0.01f, 0), size, color);
				DebugRenderer.Flush();
			}
		}
	}

}

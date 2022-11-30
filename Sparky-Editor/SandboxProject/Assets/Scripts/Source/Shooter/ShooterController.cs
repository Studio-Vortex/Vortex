using Sparky;

namespace Sandbox {

	public class ShooterController : Entity
	{
		public float moveSpeed = 5_000.0f;
		public float horizontalDeadzone = 0.1f;

		RigidBody rb;

		protected override void OnCreate()
		{
			rb = GetComponent<RigidBody>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			MovePlayer();
		}

		private void MovePlayer()
		{
			Vector3 speed = Vector3.Zero;

			float axisLeftX = Input.GetGamepadAxis(Gamepad.AxisLeftX);

			if (Input.IsKeyDown(KeyCode.A) || Input.IsKeyDown(KeyCode.Left) || axisLeftX < -horizontalDeadzone)
				speed = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D) || Input.IsKeyDown(KeyCode.Right) || axisLeftX > horizontalDeadzone)
				speed = Vector3.Right;

			if (Input.IsKeyDown(KeyCode.Space))
				rb.Rotate(Mathf.Deg2Rad(Vector3.Up));

			rb.AddForce(speed * moveSpeed * Time.DeltaTime);
		}
	}

}

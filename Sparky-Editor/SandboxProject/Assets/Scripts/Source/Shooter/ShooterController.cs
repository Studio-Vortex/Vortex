using Sparky;

namespace Sandbox {

	public class ShooterController : Entity
	{
		public float moveSpeed = 3_000.0f;
		public float maxLookUp = 35f;
		public float maxLookDown = -45f;
		public float movementDeadzone = 0.1f;
		public float verticalRotationSpeed = 10.0f;
		public float horizontalRotationSpeed = 20.0f;

		Vector2 mousePosLastFrame;
		Vector2 mousePosThisFrame;

		RigidBody rb;
		Entity eyes;

		protected override void OnCreate()
		{
			rb = GetComponent<RigidBody>();
			eyes = FindEntityByName("Eyes");
			Input.ShowMouseCursor = false;
		}

		protected override void OnUpdate(float deltaTime)
		{
			MoveHorizontal();
			RotateHorizontal();
			RotateVertical();

			mousePosLastFrame = mousePosThisFrame;
		}

		void MoveHorizontal()
		{
			Vector3 speed = Vector3.Zero;

			float axisLeftX = Input.GetGamepadAxis(Gamepad.AxisLeftX);

			if (Input.IsKeyDown(KeyCode.A) || Input.IsKeyDown(KeyCode.Left) || axisLeftX < -movementDeadzone)
				speed = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D) || Input.IsKeyDown(KeyCode.Right) || axisLeftX > movementDeadzone)
				speed = Vector3.Right;

			rb.AddForce(speed * moveSpeed * Time.DeltaTime);
		}

		Vector2 GetMouseDelta()
		{
			mousePosThisFrame = Input.GetMousePosition();

			return mousePosThisFrame - mousePosLastFrame;
		}

		void RotateHorizontal()
		{
			Vector2 delta = GetMouseDelta();
			rb.Rotate(0f, -delta.X * horizontalRotationSpeed * Time.DeltaTime, 0f);
		}

		void RotateVertical()
		{
			Vector2 delta = GetMouseDelta();
			Transform eyeTransform = eyes.transform;
			float deltaRoll = delta.Y * verticalRotationSpeed * Time.DeltaTime;
			eyeTransform.Rotate(deltaRoll, 0f, 0f);
			float clampedRoll = Mathf.Clamp(eyeTransform.Rotation.X, maxLookDown, maxLookUp);
			eyeTransform.Rotation = new Vector3(clampedRoll, eyeTransform.Rotation.YZ);
		}
	}

}

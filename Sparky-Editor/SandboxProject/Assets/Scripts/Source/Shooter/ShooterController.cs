using Sparky;

namespace Sandbox {

	public class ShooterController : Entity
	{
		public float moveSpeed = 3_000f;
		public float strafeSpeed = 2_000f;
		public float maxLookUp = 35f;
		public float maxLookDown = -45f;
		public float movementDeadzone = 0.1f;
		public float verticalRotationSpeed = 10f;
		public float horizontalRotationSpeed = 20f;
		public float jumpForce = 1_500f;

		Vector2 mousePosLastFrame;
		Vector2 mousePosThisFrame;

		RigidBody rigidbody;
		Entity eyes;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
			eyes = FindEntityByName("Eyes");
			Input.ShowMouseCursor = false;
		}

		protected override void OnUpdate(float deltaTime)
		{
			MovePlayer();
			Strafe();
			Jump();
			RotateHorizontal();
			RotateVertical();

			mousePosLastFrame = mousePosThisFrame;
		}

		void MovePlayer()
		{
			float axisLeftY = Input.GetGamepadAxis(Gamepad.AxisLeftY);
			Vector3 speed = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W) || axisLeftY > movementDeadzone)
				speed = transform.Forward;
			else if (Input.IsKeyDown(KeyCode.S) || axisLeftY < -movementDeadzone)
				speed = -transform.Forward;

			rigidbody.AddForce(speed * moveSpeed * Time.DeltaTime);
		}

		void Strafe()
		{
			float axisLeftX = Input.GetGamepadAxis(Gamepad.AxisLeftX);
			Vector3 speed = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.A) || axisLeftX < -movementDeadzone)
				speed = -transform.Right;
			else if (Input.IsKeyDown(KeyCode.D) || axisLeftX > movementDeadzone)
				speed = transform.Right;

			rigidbody.AddForce(speed * strafeSpeed * Time.DeltaTime);
		}

		void Jump()
		{
			bool isGrounded = false;
			if (transform.Translation.Y == 1.5f)
				isGrounded = true;

			if (Input.IsKeyDown(KeyCode.Space) && isGrounded)
				rigidbody.AddForce(Vector3.Up * jumpForce * Time.DeltaTime, ForceMode.Impulse);
		}

		Vector2 GetMouseDelta()
		{
			mousePosThisFrame = Input.GetMousePosition();

			return mousePosThisFrame - mousePosLastFrame;
		}

		void RotateHorizontal()
		{
			Vector2 delta = GetMouseDelta();
			rigidbody.Rotate(0f, -delta.X * horizontalRotationSpeed * Time.DeltaTime, 0f);
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

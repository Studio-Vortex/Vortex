using Sparky;

namespace Sandbox {

	public class ShooterController : Entity
	{
		public float walkSpeed = 3f;
		public float runSpeed = 6f;
		public float strafeSpeed = 2f;
		public float maxLookUp = 35f;
		public float maxLookDown = -45f;
		public float movementDeadzone = 0.1f;
		public float verticalRotationSpeed = 10f;
		public float horizontalRotationSpeed = 20f;
		public float jumpForce = 5f;

		Vector2 mousePosLastFrame;
		Vector2 mousePosThisFrame;

		RigidBody rigidbody;
		CapsuleCollider capsuleCollider;
		CharacterController controller;
		Entity eyes;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
			controller = GetComponent<CharacterController>();
			capsuleCollider = GetComponent<CapsuleCollider>();
			eyes = FindEntityByName("Eyes");
			Input.ShowMouseCursor = false;
			rigidbody.Rotation = Vector3.Zero;
			eyes.transform.Rotation = Vector3.Zero;
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

			if (Input.IsKeyDown(KeyCode.W) || axisLeftY < -movementDeadzone)
				speed = transform.Forward;
			else if (Input.IsKeyDown(KeyCode.S) || axisLeftY > movementDeadzone)
				speed = -transform.Forward;

			float modifier = Input.IsKeyDown(KeyCode.LeftShift) ? runSpeed : walkSpeed;

			controller.Move(speed * modifier * Time.DeltaTime);
		}

		void Strafe()
		{
			float axisLeftX = Input.GetGamepadAxis(Gamepad.AxisLeftX);
			Vector3 speed = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.A) || axisLeftX < -movementDeadzone)
				speed = -transform.Right;
			else if (Input.IsKeyDown(KeyCode.D) || axisLeftX > movementDeadzone)
				speed = transform.Right;

			controller.Move(speed * strafeSpeed * Time.DeltaTime);
		}

		void Jump()
		{
			

			if (Input.IsKeyDown(KeyCode.Space) && controller.IsGrounded)
				controller.Jump(jumpForce);
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

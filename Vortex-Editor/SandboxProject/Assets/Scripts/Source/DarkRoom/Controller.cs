using Vortex;

namespace Sandbox.DarkRoom {

	public class Controller : Entity
	{
		public float moveSpeed = 5f;
		public float jumpForce = 5f;
		public float maxLookUp = 60f;
		public float maxLookDown = -60f;
		public float verticalRotationSpeed = 10f;
		public float horizontalRotationSpeed = 20f;

		Vector2 mousePosThisFrame;
		Vector2 mousePosLastFrame;
		CharacterController controller;
		AudioSource[] footstepSounds;
		RigidBody rigidbody;
		Entity eyes;
		bool inputEnabled = true;

		protected override void OnCreate()
		{
			controller = GetComponent<CharacterController>();
			rigidbody = GetComponent<RigidBody>();
			eyes = Scene.FindEntityByName("Eyes");
			footstepSounds = new AudioSource[2];
			footstepSounds[0] = Scene.FindEntityByName("Footstep1").GetComponent<AudioSource>();
			footstepSounds[1] = Scene.FindEntityByName("Footstep2").GetComponent<AudioSource>();
			Input.SetCursorMode(CursorMode.Locked);
		}

		protected override void OnUpdate(float delta)
		{
			if (inputEnabled)
			{
				MovePlayer();
				RotatePlayer();
				Jump();
			}

			if (Input.IsKeyDown(KeyCode.Escape))
			{
				inputEnabled = false;
				Input.SetCursorMode(CursorMode.Normal);
			}
			else if (Input.IsKeyDown(KeyCode.F6))
			{
				inputEnabled = true;
				Input.SetCursorMode(CursorMode.Locked);
			}
		}

		bool FootstepSoundIsPlaying()
		{
			return footstepSounds[0].IsPlaying || footstepSounds[1].IsPlaying;
		}

		void MovePlayer()
		{
			Vector3 displacement = Vector3.Zero;

			int rand0 = Random.Int(0, 1);
			int rand1 = Random.Int(0, 1);

			if (Input.IsKeyDown(KeyCode.W))
			{
				displacement = transform.Forward;
				
				if (controller.IsGrounded && !FootstepSoundIsPlaying())
					footstepSounds[rand0].Play();
			}
			else if (Input.IsKeyDown(KeyCode.S))
			{
				displacement = -transform.Forward;
				
				if (controller.IsGrounded && !FootstepSoundIsPlaying())
					footstepSounds[rand0].Play();
			}
			if (Input.IsKeyDown(KeyCode.A))
			{
				displacement = -transform.Right;
				
				if (controller.IsGrounded && !FootstepSoundIsPlaying())
					footstepSounds[rand1].Play();
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				displacement = transform.Right;
				
				if (controller.IsGrounded && !FootstepSoundIsPlaying())
					footstepSounds[rand1].Play();
			}

			float modifier = Input.IsKeyDown(KeyCode.LeftShift) ? moveSpeed * 2 : moveSpeed;
			controller.Move(displacement * modifier * Time.DeltaTime);
		}

		void RotatePlayer()
		{
			mousePosThisFrame = Input.MousePosition;
			Vector2 mouseDelta = mousePosThisFrame - mousePosLastFrame;
			mousePosLastFrame = mousePosThisFrame;

			float deltaRoll = mouseDelta.Y * verticalRotationSpeed * Time.DeltaTime;
			float deltaPitch = -mouseDelta.X * horizontalRotationSpeed * Time.DeltaTime;
			rigidbody.Rotate(0, deltaPitch, 0);
			eyes.transform.Rotate(deltaRoll, 0, 0);
			float clampedRoll = Mathf.Clamp(eyes.transform.Rotation.X, maxLookDown, maxLookUp);
			eyes.transform.Rotation = new Vector3(clampedRoll, eyes.transform.Rotation.YZ);
		}

		void Jump()
		{
			if (Input.IsKeyDown(KeyCode.Space) && controller.IsGrounded)
				controller.Jump(jumpForce);
		}
	}

}

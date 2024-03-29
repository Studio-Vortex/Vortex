﻿using Vortex;

namespace Sandbox.Shooter {

	public class ShooterController : Entity
	{
		public float walkSpeed = 3f;
		public float runSpeed = 6f;
		public float strafeSpeed = 2f;
		public float runStrafeSpeed = 4f;
		public float maxLookUp = 35f;
		public float maxLookDown = -45f;
		public float controllerDeadzone = 0.15f;
		public float mouseVerticalRotationSpeed = 10f;
		public float mouseHorizontalRotationSpeed = 20f;
		public float controllerVerticalRotationSpeed = 100f;
		public float controllerHorizontalRotationSpeed = 200f;
		public float jumpForce = 6f;

		Vector2 mousePosLastFrame;
		Vector2 mousePosThisFrame;

		AudioSource[] footstepSounds;
		RigidBody rigidbody;
		CapsuleCollider capsuleCollider;
		CharacterController controller;
		Entity eyes;

		bool isGrounded = false;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
			controller = GetComponent<CharacterController>();
			capsuleCollider = GetComponent<CapsuleCollider>();
			eyes = Scene.FindEntityByName("Eyes");
			Input.SetCursorMode(CursorMode.Locked);
			transform.Rotation = Quaternion.Identity;
			eyes.transform.Rotation = Quaternion.Identity;
			footstepSounds = new AudioSource[2];
			footstepSounds[0] = Scene.FindEntityByName("Footstep 1").GetComponent<AudioSource>();
			footstepSounds[1] = Scene.FindEntityByName("Footstep 2").GetComponent<AudioSource>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			MovePlayer();
			RotatePlayer();
			QuitOrReloadLevelIfNeeded();
		}

		static void QuitOrReloadLevelIfNeeded()
		{
			if (Input.IsKeyDown(KeyCode.Escape) || Input.IsGamepadButtonDown(GamepadButton.Start))
			{
				Application.Quit();
			}

			if (Input.IsKeyDown(KeyCode.F1) || Input.IsGamepadButtonDown(GamepadButton.Guide))
			{
				SceneManager.LoadScene("Shooter - Level01");
			}
		}

		void RotatePlayer()
		{
			mousePosThisFrame = Input.MousePosition;
			Vector2 mouseDelta = mousePosThisFrame - mousePosLastFrame;
			mousePosLastFrame = mousePosThisFrame;

			// Horizontal
			transform.Rotate(0f, -mouseDelta.X * mouseHorizontalRotationSpeed * Time.DeltaTime, 0f);

			float rightAxisX = Input.GetGamepadAxis(GamepadAxis.RightX);
			if (rightAxisX < -controllerDeadzone || rightAxisX > controllerDeadzone)
			{
				transform.Rotate(0f, -rightAxisX * controllerHorizontalRotationSpeed * Time.DeltaTime, 0f);
			}

			// Vertical
			{
				Transform eyeTransform = eyes.transform;
				float deltaRoll = mouseDelta.Y * mouseVerticalRotationSpeed * Time.DeltaTime;
				eyeTransform.Rotate(deltaRoll, 0f, 0f);
				float clampedRoll = Mathf.Clamp(eyeTransform.Rotation.X, maxLookDown, maxLookUp);
				eyeTransform.Rotation = new Quaternion(new Vector3(clampedRoll, eyeTransform.EulerAngles.YZ));
			}

			{
				float rightAxisY = Input.GetGamepadAxis(GamepadAxis.RightY);
				if (rightAxisY < -controllerDeadzone || rightAxisY > controllerDeadzone)
				{
					Transform eyeTransform = eyes.transform;
					float deltaRoll = -rightAxisY * controllerVerticalRotationSpeed * Time.DeltaTime;
					eyeTransform.Rotate(deltaRoll, 0f, 0f);
					float clampedRoll = Mathf.Clamp(eyeTransform.Rotation.X, maxLookDown, maxLookUp);
					eyeTransform.Rotation = new Quaternion(new Vector3(clampedRoll, eyeTransform.EulerAngles.YZ));
				}
			}
		}

		void MovePlayer()
		{
			MoveForwardBackward();
			Strafe();
			Jump();
		}

		bool FootstepSoundPlaying()
		{
			return footstepSounds[0].IsPlaying || footstepSounds[1].IsPlaying;
		}

		void MoveForwardBackward()
		{
			float axisLeftY = Input.GetGamepadAxis(GamepadAxis.LeftY);
			Vector3 speed = Vector3.Zero;

			int randomInt = Random.Int(0, 1);

			if (Input.IsKeyDown(KeyCode.W) || axisLeftY < -controllerDeadzone)
			{
				speed += transform.Forward;
				
				if (isGrounded && !FootstepSoundPlaying())
					footstepSounds[randomInt].Play();
			}
			else if (Input.IsKeyDown(KeyCode.S) || axisLeftY > controllerDeadzone)
			{
				speed += -transform.Forward;
				
				if (isGrounded && !FootstepSoundPlaying())
					footstepSounds[randomInt].Play();
			}

			bool leftShiftPressed = Input.IsKeyDown(KeyCode.LeftShift);
			bool leftStickPressed = Input.IsGamepadButtonDown(GamepadButton.LeftStick);

			float modifier = (leftShiftPressed || leftStickPressed) ? runSpeed : walkSpeed;
			controller.Move(speed * modifier * Time.DeltaTime);
		}

		void Strafe()
		{
			float axisLeftX = Input.GetGamepadAxis(GamepadAxis.LeftX);
			Vector3 speed = Vector3.Zero;

			int randomInt = Random.Int(0, 1);

			if (Input.IsKeyDown(KeyCode.A) || axisLeftX < -controllerDeadzone)
			{
				speed += -transform.Right;
				
				if (isGrounded && !FootstepSoundPlaying())
					footstepSounds[randomInt].Play();
			}
			else if (Input.IsKeyDown(KeyCode.D) || axisLeftX > controllerDeadzone)
			{
				speed += transform.Right;
				
				if (isGrounded && !FootstepSoundPlaying())
					footstepSounds[randomInt].Play();
			}

			bool leftShiftPressed = Input.IsKeyDown(KeyCode.LeftShift);
			bool leftStickPressed = Input.IsGamepadButtonDown(GamepadButton.LeftStick);

			float modifier = (leftShiftPressed || leftStickPressed) ? runStrafeSpeed : strafeSpeed;
			controller.Move(speed * modifier * Time.DeltaTime);
		}

		void Jump()
		{
			bool spacePressed = Input.IsKeyDown(KeyCode.Space);
			bool aButtonPressed = Input.IsGamepadButtonDown(GamepadButton.A);

			isGrounded = Physics.Raycast(transform.Translation + Vector3.Down * capsuleCollider.Height, Vector3.Down, 0.05f, out RaycastHit hitInfo);

			if ((spacePressed || aButtonPressed) && isGrounded)
				controller.Jump(jumpForce);
		}
	}

}

﻿using Sparky;

namespace Sandbox {

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
			eyes = FindEntityByName("Eyes");
			Input.ShowMouseCursor = false;
			rigidbody.Rotation = Vector3.Zero;
			eyes.transform.Rotation = Vector3.Zero;
			footstepSounds = new AudioSource[2];
			footstepSounds[0] = FindEntityByName("Footstep 1").GetComponent<AudioSource>();
			footstepSounds[1] = FindEntityByName("Footstep 2").GetComponent<AudioSource>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			MovePlayer();
			Strafe();
			Jump();
			RotateHorizontal();
			RotateVertical();

			if (Input.IsKeyDown(KeyCode.Escape) || Input.IsGamepadButtonDown(Gamepad.ButtonStart))
			{
				Application.Quit();
			}

			if (Input.IsKeyDown(KeyCode.F1) || Input.IsGamepadButtonDown(Gamepad.ButtonGuide))
			{
				SceneManager.LoadScene("FPSDemoRevamped");
			}

			mousePosLastFrame = mousePosThisFrame;
		}

		void MovePlayer()
		{
			float axisLeftY = Input.GetGamepadAxis(Gamepad.AxisLeftY);
			Vector3 speed = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W) || axisLeftY < -controllerDeadzone)
			{
				speed = transform.Forward;
				
				if (isGrounded)
					footstepSounds[0].Play();
			}
			else if (Input.IsKeyDown(KeyCode.S) || axisLeftY > controllerDeadzone)
			{
				speed = -transform.Forward;
				
				if (isGrounded)
					footstepSounds[1].Play();
			}

			bool leftShiftPressed = Input.IsKeyDown(KeyCode.LeftShift);
			bool leftStickPressed = Input.IsGamepadButtonDown(Gamepad.LeftStick);

			float modifier = (leftShiftPressed || leftStickPressed) ? runSpeed : walkSpeed;
			controller.Move(speed * modifier * Time.DeltaTime);
		}

		void Strafe()
		{
			float axisLeftX = Input.GetGamepadAxis(Gamepad.AxisLeftX);
			Vector3 speed = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.A) || axisLeftX < -controllerDeadzone)
			{
				speed = -transform.Right;
				
				if (isGrounded)
					footstepSounds[0].Play();
			}
			else if (Input.IsKeyDown(KeyCode.D) || axisLeftX > controllerDeadzone)
			{
				speed = transform.Right;
				
				if (isGrounded)
					footstepSounds[1].Play();
			}

			bool leftShiftPressed = Input.IsKeyDown(KeyCode.LeftShift);
			bool leftStickPressed = Input.IsGamepadButtonDown(Gamepad.LeftStick);

			float modifier = (leftShiftPressed || leftStickPressed) ? runStrafeSpeed : strafeSpeed;
			controller.Move(speed * modifier * Time.DeltaTime);
		}

		void Jump()
		{
			bool spacePressed = Input.IsKeyDown(KeyCode.Space);
			bool aButtonPressed = Input.IsGamepadButtonDown(Gamepad.ButtonA);

			isGrounded = Physics.Raycast(transform.Translation + Vector3.Down * capsuleCollider.Height, Vector3.Down, 0.05f, out RaycastHit hitInfo);

			if ((spacePressed || aButtonPressed) && isGrounded)
				controller.Jump(jumpForce);
		}

		Vector2 GetMouseDelta()
		{
			mousePosThisFrame = Input.GetMousePosition();

			return mousePosThisFrame - mousePosLastFrame;
		}

		void RotateHorizontal()
		{
			Vector2 mouseDelta = GetMouseDelta();
			rigidbody.Rotate(0f, -mouseDelta.X * mouseHorizontalRotationSpeed * Time.DeltaTime, 0f);

			float rightAxisX = Input.GetGamepadAxis(Gamepad.AxisRightX);
			if (rightAxisX < -controllerDeadzone || rightAxisX > controllerDeadzone)
			{
				rigidbody.Rotate(0f, -rightAxisX * controllerHorizontalRotationSpeed * Time.DeltaTime, 0f);
			}
		}

		void RotateVertical()
		{
			{
				Vector2 mouseDelta = GetMouseDelta();
				Transform eyeTransform = eyes.transform;
				float deltaRoll = mouseDelta.Y * mouseVerticalRotationSpeed * Time.DeltaTime;
				eyeTransform.Rotate(deltaRoll, 0f, 0f);
				float clampedRoll = Mathf.Clamp(eyeTransform.Rotation.X, maxLookDown, maxLookUp);
				eyeTransform.Rotation = new Vector3(clampedRoll, eyeTransform.Rotation.YZ);
			}

			{
				float rightAxisY = Input.GetGamepadAxis(Gamepad.AxisRightY);
				if (rightAxisY < -controllerDeadzone || rightAxisY > controllerDeadzone)
				{
					Transform eyeTransform = eyes.transform;
					float deltaRoll = -rightAxisY * controllerVerticalRotationSpeed * Time.DeltaTime;
					eyeTransform.Rotate(deltaRoll, 0f, 0f);
					float clampedRoll = Mathf.Clamp(eyeTransform.Rotation.X, maxLookDown, maxLookUp);
					eyeTransform.Rotation = new Vector3(clampedRoll, eyeTransform.Rotation.YZ);
				}
			}
		}
	}

}

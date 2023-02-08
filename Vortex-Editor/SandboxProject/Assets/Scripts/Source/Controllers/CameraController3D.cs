﻿using Vortex;

namespace Sandbox {

	public class CameraController3D : Entity
	{
		public float MaxRoll_Up = 60.0f;
		public float MaxRoll_Down = -60.0f;
		public float ShiftModifer = 2.0f;
		public float ControllerDeadzone = 0.15f;
		public Vector3 WalkSpeed = new Vector3(4.0f, 0.0f, 4.0f);
		public Vector3 RunSpeed = new Vector3(10.0f, 0.0f, 10.0f);
		public Vector3 RotationSpeed = new Vector3(100.0f, 100.0f, 0.0f);
		
		public bool FixedRotation;

		private Vector3 m_Velocity;
		private Vector3 m_Rotation;

		void OnUpdate(float delta)
		{
			Vector3 speed = Input.IsGamepadButtonDown(Gamepad.LeftStick) ? RunSpeed : WalkSpeed;

			if (Input.IsGamepadButtonDown(Gamepad.ButtonStart))
				Application.Quit();

			if (Input.GetGamepadAxis(Gamepad.AxisLeftY) < -ControllerDeadzone)
				transform.Translate(-transform.Forward * speed * delta * Input.GetGamepadAxis(Gamepad.AxisLeftY));
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftY) > ControllerDeadzone)
				transform.Translate(-transform.Forward * speed * delta * Input.GetGamepadAxis(Gamepad.AxisLeftY));

			if (Input.GetGamepadAxis(Gamepad.AxisLeftX) < -ControllerDeadzone)
				transform.Translate(transform.Right * speed * delta * Input.GetGamepadAxis(Gamepad.AxisLeftX));
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftX) > ControllerDeadzone)
				transform.Translate(transform.Right * speed * delta * Input.GetGamepadAxis(Gamepad.AxisLeftX));

			if (Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > ControllerDeadzone)
				m_Velocity.Y = 1.0f * Input.GetGamepadAxis(Gamepad.AxisRightTrigger);
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > ControllerDeadzone)
				m_Velocity.Y = -1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftTrigger);

			if (!FixedRotation)
				ProcessRotation();

			if (Input.IsGamepadButtonDown(Gamepad.LeftStick))
				m_Velocity *= ShiftModifer;

			m_Velocity *= speed * delta;
			m_Rotation *= RotationSpeed * delta;

			transform.Translation += m_Velocity;
			transform.Rotation += m_Rotation;
		}

		void ProcessRotation()
		{
			float rightAxisX = -Input.GetGamepadAxis(Gamepad.AxisRightX);
			if (rightAxisX < -ControllerDeadzone || rightAxisX > ControllerDeadzone)
				m_Rotation.Y = rightAxisX;

			float rightAxisY = -Input.GetGamepadAxis(Gamepad.AxisRightY);
			if (rightAxisY < -ControllerDeadzone || rightAxisY > ControllerDeadzone)
				m_Rotation.X = rightAxisY;

			float roll = Mathf.Clamp(transform.Rotation.X, MaxRoll_Down, MaxRoll_Up);
			transform.Rotation = new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z);
		}
	}

}

using System;
using Sparky;

namespace Sandbox {

	public class CameraController3D : Entity
	{
		public float MaxRoll_Up = 60.0f;
		public float MaxRoll_Down = -60.0f;
		public float ShiftModifer = 2.0f;
		public float ControllerDeadzone = 0.15f;
		public Vector3 Speed = new Vector3(4.0f, 0.0f, 4.0f);
		public Vector3 RotationSpeed = new Vector3(100.0f, 100.0f, 0.0f);
		
		public bool FixedRotation;

		private Vector3 m_Velocity;
		private Vector3 m_Rotation;

		public override void OnCreate()
		{
			Window.ShowMouseCursor(false);
		}

		public override void OnUpdate(float delta)
		{
			if (Input.GetGamepadAxis(Gamepad.AxisLeftY) < -ControllerDeadzone)
				m_Velocity.Z = -1.0f * -Input.GetGamepadAxis(Gamepad.AxisLeftY);
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftY) > ControllerDeadzone)
				m_Velocity.Z = 1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftY);

			if (Input.GetGamepadAxis(Gamepad.AxisLeftX) < -ControllerDeadzone)
				m_Velocity.X = -1.0f * -Input.GetGamepadAxis(Gamepad.AxisLeftX);
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftX) > ControllerDeadzone)
				m_Velocity.X = 1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftX);

			if (Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > ControllerDeadzone)
				m_Velocity.Y = 1.0f * Input.GetGamepadAxis(Gamepad.AxisRightTrigger);
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > ControllerDeadzone)
				m_Velocity.Y = -1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftTrigger);

			if (!FixedRotation)
				ProcessRotation();

			if (Input.IsKeyDown(KeyCode.LeftShift) || Input.IsGamepadButtonDown(Gamepad.LeftStick))
				m_Velocity *= ShiftModifer;

			m_Velocity *= Speed * delta;
			m_Rotation *= RotationSpeed * delta;

			transform.Translation += m_Velocity;
			transform.Rotation += m_Rotation;
		}

		private void ProcessRotation()
		{
			if (Input.GetGamepadAxis(Gamepad.AxisRightX) < -ControllerDeadzone)
				m_Rotation.Y = 1.0f * -Input.GetGamepadAxis(Gamepad.AxisRightX);
			if (Input.GetGamepadAxis(Gamepad.AxisRightX) > ControllerDeadzone)
				m_Rotation.Y = -1.0f * Input.GetGamepadAxis(Gamepad.AxisRightX);

			if (Input.GetGamepadAxis(Gamepad.AxisRightY) < -ControllerDeadzone)
				m_Rotation.X = 1.0f * -Input.GetGamepadAxis(Gamepad.AxisRightY);
			if (Input.GetGamepadAxis(Gamepad.AxisRightY) > ControllerDeadzone)
				m_Rotation.X = -1.0f * Input.GetGamepadAxis(Gamepad.AxisRightY);

			if (transform.Rotation.X >= MaxRoll_Up)
			{
				float roll = Math.Min(MaxRoll_Up, transform.Rotation.X);
				transform.Rotation = new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z);
			}
			if (transform.Rotation.X <= MaxRoll_Down)
			{
				float roll = Math.Max(MaxRoll_Down, transform.Rotation.X);
				transform.Rotation = new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z);
			}
		}
	}

}

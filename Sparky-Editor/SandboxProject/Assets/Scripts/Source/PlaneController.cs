using System;
using Sparky;

namespace Sandbox {

	public class PlaneController : Entity
	{
		public float MoveSpeed;
		public float RotationSpeed;
		public Vector3 CameraOffset;
		public float Deadzone = 0.15f;
		public Vector3 Forward;
		Entity m_Camera;

		public override void OnCreate()
		{
			m_Camera = FindEntityByName("Camera");
		}

		public override void OnUpdate(float delta)
		{
			Forward = transform.Forward;
			ProcessMovement(delta);
			ProcessRotation(delta);

			m_Camera.transform.Translation = transform.Translation + CameraOffset;
		}

		void ProcessMovement(float delta)
		{
			if (Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > Deadzone)
				transform.Translate(transform.Forward * MoveSpeed * delta);
			if (Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > Deadzone)
				transform.Translate(transform.Forward * -MoveSpeed * delta);
		}

		void ProcessRotation(float delta)
		{
			if (Input.GetGamepadAxis(Gamepad.AxisLeftX) > Deadzone)
				transform.Rotate(0, 0, -RotationSpeed * delta);
			if (Input.GetGamepadAxis(Gamepad.AxisLeftX) < -Deadzone)
				transform.Rotate(0, 0, RotationSpeed * delta);

			if (Input.GetGamepadAxis(Gamepad.AxisLeftY) > Deadzone)
				transform.Rotate(RotationSpeed * delta, 0, 0);
			if (Input.GetGamepadAxis(Gamepad.AxisLeftY) < -Deadzone)
				transform.Rotate(-RotationSpeed * delta, 0, 0);
		}
	}

}

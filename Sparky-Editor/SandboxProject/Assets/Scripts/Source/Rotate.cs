using System;
using Sparky;

namespace Sandbox {

	public class Rotate : Entity
	{
		public float Speed;
		public float ControllerDeadzone;
		public Vector3 Velocity;

		public override void OnUpdate(float delta)
		{
			Velocity = Vector3.Zero;

			if (Input.GetGamepadAxis(Gamepad.AxisLeftY) < -ControllerDeadzone)
				Velocity.Y = -Input.GetGamepadAxis(Gamepad.AxisLeftY);
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftY) > ControllerDeadzone)
				Velocity.Y = -Input.GetGamepadAxis(Gamepad.AxisLeftY);

			if (Input.GetGamepadAxis(Gamepad.AxisLeftX) < -ControllerDeadzone)
				Velocity.X = Input.GetGamepadAxis(Gamepad.AxisLeftX);
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftX) > ControllerDeadzone)
				Velocity.X = Input.GetGamepadAxis(Gamepad.AxisLeftX);

			if (Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > ControllerDeadzone)
				Velocity.Z = Input.GetGamepadAxis(Gamepad.AxisLeftTrigger);
			else if (Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > ControllerDeadzone)
				Velocity.Z = -Input.GetGamepadAxis(Gamepad.AxisRightTrigger);

			transform.Translate(Velocity * Speed * delta);
			transform.Rotate(new Vector3(Speed * delta) * 6.0f);
		}
	}

}

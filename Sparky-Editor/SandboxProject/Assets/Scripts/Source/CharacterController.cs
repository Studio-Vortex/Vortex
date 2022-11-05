using System;
using System.Security.Permissions;
using Sparky;

namespace Sandbox {

	public class CharacterController : Entity
	{
		public float WalkSpeed;
		public float RunSpeed;
		public float Deadzone = 0.15f;

		public override void OnCreate()
		{
		}

		public override void OnUpdate(float delta)
		{
			Vector3 velocity = Vector3.Zero;

			float leftX = Input.GetGamepadAxis(Gamepad.AxisLeftX);
			if (leftX > Deadzone || leftX < -Deadzone)
				velocity.X = leftX;

			float leftY = Input.GetGamepadAxis(Gamepad.AxisLeftY);
			if (leftY > Deadzone || leftY < -Deadzone)
				velocity.Z = leftY;

			transform.Translate(velocity * delta * WalkSpeed);
		}
	}

}

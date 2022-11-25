using System;
using Sparky;

namespace Sandbox {

	public class PlaneController : Entity
	{
		public float MoveSpeed;
		public float RotationSpeed;
		public float Deadzone = 0.15f;
		public Vector3 CameraOffset;
		public Vector3 Velocity;
		public Vector3 Rotation;

		protected override void OnCreate()
		{

		}

		protected override void OnUpdate(float delta)
		{
			Debug.Log($"Forward: {transform.Forward}");
			Debug.Log($"Right:   {transform.Right}");
			Debug.Log($"Up:      {transform.Up}");
			Velocity = Vector3.Zero;
			Rotation = Vector3.Zero;

			ProcessMovement(delta);
			ProcessRotation(delta);

			transform.Translate(Velocity);
			transform.Rotate(Rotation);
		}

		void ProcessMovement(float delta)
		{
			float axisLeftTrigger = Input.GetGamepadAxis(Gamepad.AxisLeftTrigger);
			if (axisLeftTrigger > Deadzone)
				Velocity = transform.Forward * MoveSpeed * delta * axisLeftTrigger;

			float axisRightTrigger = Input.GetGamepadAxis(Gamepad.AxisRightTrigger);
			if (axisRightTrigger > Deadzone)
				Velocity = transform.Forward * MoveSpeed * delta * axisRightTrigger;
		}

		void ProcessRotation(float delta)
		{
			// left right rotation
			float axisLeftX = Input.GetGamepadAxis(Gamepad.AxisLeftX);
			if (axisLeftX > Deadzone || axisLeftX < -Deadzone)
				Rotation = transform.Forward * RotationSpeed * delta * axisLeftX;

			// up down rotation
			float axisLeftY = Input.GetGamepadAxis(Gamepad.AxisLeftY);
			if (axisLeftY > Deadzone || axisLeftY < -Deadzone)
				Rotation = transform.Right * RotationSpeed * delta * axisLeftY;

			if (Input.IsGamepadButtonDown(Gamepad.LeftBumper))
				Rotation = transform.Up * RotationSpeed * delta;
			if (Input.IsGamepadButtonDown(Gamepad.RightBumper))
				Rotation = transform.Up * -RotationSpeed * delta;
		}
	}

}

using Vortex;

namespace Sandbox {

	public class PlaneController : Entity
	{
		public float MoveSpeed;
		public float RotationSpeed;
		public float Deadzone = 0.15f;
		public Vector3 CameraOffset;
		public Vector3 Velocity;
		public Vector3 Rotation;

		protected override void OnUpdate(float delta)
		{
			Velocity = Vector3.Zero;
			Rotation = Vector3.Zero;

			ProcessMovement(delta);
			ProcessRotation(delta);

			transform.Translate(Velocity);
			transform.Rotate(Rotation);
		}

		void ProcessMovement(float delta)
		{
			float axisLeftTrigger = Input.GetGamepadAxis(GamepadAxis.LeftTrigger);
			if (axisLeftTrigger > Deadzone)
				Velocity = transform.Forward * MoveSpeed * delta * axisLeftTrigger;

			float axisRightTrigger = Input.GetGamepadAxis(GamepadAxis.LeftTrigger);
			if (axisRightTrigger > Deadzone)
				Velocity = transform.Forward * MoveSpeed * delta * axisRightTrigger;
		}

		void ProcessRotation(float delta)
		{
			// left right rotation
			float axisLeftX = Input.GetGamepadAxis(GamepadAxis.LeftTrigger);
			if (axisLeftX > Deadzone || axisLeftX < -Deadzone)
				Rotation = transform.Forward * RotationSpeed * delta * axisLeftX;

			// up down rotation
			float axisLeftY = Input.GetGamepadAxis(GamepadAxis.LeftTrigger);
			if (axisLeftY > Deadzone || axisLeftY < -Deadzone)
				Rotation = transform.Right * RotationSpeed * delta * axisLeftY;

			if (Input.IsGamepadButtonDown(GamepadButton.LeftBumper))
				Rotation = transform.Up * RotationSpeed * delta;
			if (Input.IsGamepadButtonDown(GamepadButton.RightBumper))
				Rotation = transform.Up * -RotationSpeed * delta;
		}
	}

}

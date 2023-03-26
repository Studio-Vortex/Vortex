using Vortex;

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

		private Vector3 Velocity;
		private Vector3 Rotation;

		void OnUpdate(float delta)
		{
			Vector3 speed = Input.IsGamepadButtonDown(GamepadButton.LeftStick) ? RunSpeed : WalkSpeed;

			if (Input.IsGamepadButtonDown(GamepadButton.Start))
				Application.Quit();

			if (Input.GetGamepadAxis(GamepadAxis.LeftY) < -ControllerDeadzone)
				Velocity += -transform.Forward * Input.GetGamepadAxis(GamepadAxis.LeftY);
			else if (Input.GetGamepadAxis(GamepadAxis.LeftY) > ControllerDeadzone)
				Velocity += -transform.Forward * Input.GetGamepadAxis(GamepadAxis.LeftY);

			if (Input.GetGamepadAxis(GamepadAxis.LeftX) < -ControllerDeadzone)
				Velocity += transform.Right * Input.GetGamepadAxis(GamepadAxis.LeftX);
			else if (Input.GetGamepadAxis(GamepadAxis.LeftX) > ControllerDeadzone)
				Velocity += transform.Right * Input.GetGamepadAxis(GamepadAxis.LeftX);

			if (Input.GetGamepadAxis(GamepadAxis.RightTrigger) > ControllerDeadzone)
				Velocity.Y += 1.0f * Input.GetGamepadAxis(GamepadAxis.RightTrigger);
			else if (Input.GetGamepadAxis(GamepadAxis.LeftTrigger) > ControllerDeadzone)
				Velocity.Y += -1.0f * Input.GetGamepadAxis(GamepadAxis.LeftTrigger);

			ProcessRotation();

			Velocity *= speed * delta;
			Rotation *= RotationSpeed * delta;

			transform.Translation += Velocity;
			transform.Rotation *= new Quaternion(Rotation);
		}

		void ProcessRotation()
		{
			float rightAxisX = -Input.GetGamepadAxis(GamepadAxis.RightX);
			if (rightAxisX < -ControllerDeadzone || rightAxisX > ControllerDeadzone)
				Rotation.Y = rightAxisX;

			float rightAxisY = -Input.GetGamepadAxis(GamepadAxis.RightY);
			if (rightAxisY < -ControllerDeadzone || rightAxisY > ControllerDeadzone)
				Rotation.X = rightAxisY;

			float roll = Mathf.Clamp(transform.Rotation.X, MaxRoll_Down, MaxRoll_Up);
			transform.Rotation *= new Quaternion(new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z));
		}
	}

}

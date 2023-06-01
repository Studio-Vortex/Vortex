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

		Vector3 Velocity;

		void OnUpdate(float delta)
		{
			if (Input.IsGamepadButtonDown(GamepadButton.Start))
				Application.Quit();
			
			ProcessMovement();
			ProcessRotation();
		}

		void ProcessMovement()
		{
			if (Input.GetGamepadAxis(GamepadAxis.LeftY) < -ControllerDeadzone)
				Velocity += -transform.Forward * Input.GetGamepadAxis(GamepadAxis.LeftY);
			else if (Input.GetGamepadAxis(GamepadAxis.LeftY) > ControllerDeadzone)
				Velocity += -transform.Forward * Input.GetGamepadAxis(GamepadAxis.LeftY);

			if (Input.GetGamepadAxis(GamepadAxis.LeftX) < -ControllerDeadzone)
				Velocity += transform.Right * Input.GetGamepadAxis(GamepadAxis.LeftX);
			else if (Input.GetGamepadAxis(GamepadAxis.LeftX) > ControllerDeadzone)
				Velocity += transform.Right * Input.GetGamepadAxis(GamepadAxis.LeftX);

			if (Input.GetGamepadAxis(GamepadAxis.RightTrigger) > ControllerDeadzone)
				Velocity.Y += Input.GetGamepadAxis(GamepadAxis.RightTrigger);
			else if (Input.GetGamepadAxis(GamepadAxis.LeftTrigger) > ControllerDeadzone)
				Velocity.Y += -Input.GetGamepadAxis(GamepadAxis.LeftTrigger);

			Vector3 speed = Input.IsGamepadButtonDown(GamepadButton.LeftStick) ? RunSpeed : WalkSpeed;
			Velocity *= speed * Time.DeltaTime;
			transform.Translation += Velocity;
		}

		void ProcessRotation()
		{
			float rightAxisX = -Input.GetGamepadAxis(GamepadAxis.RightX);
			if (rightAxisX < -ControllerDeadzone || rightAxisX > ControllerDeadzone)
				transform.Rotation *= Quaternion.AngleAxis(rightAxisX, Vector3.Up);

			float rightAxisY = -Input.GetGamepadAxis(GamepadAxis.RightY);
			if (rightAxisY < -ControllerDeadzone || rightAxisY > ControllerDeadzone)
				transform.Rotation *= Quaternion.AngleAxis(rightAxisY, Vector3.Right);
		}
	}

}

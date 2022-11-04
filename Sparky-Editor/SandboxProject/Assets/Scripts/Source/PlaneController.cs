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
		Entity m_Camera;
		Entity m_PointLight;

		public override void OnCreate()
		{
			m_Camera = FindEntityByName("Camera");
			m_PointLight = FindEntityByName("Point Light");
		}

		public override void OnUpdate(float delta)
		{
			if (Input.IsGamepadButtonDown(Gamepad.ButtonStart))
				Scene.Pause();

			m_Camera.transform.Translation = transform.Translation + CameraOffset;
			//m_Camera.transform.LookAt(transform.Translation);
			//m_Camera.transform.Rotation = transform.Rotation * 0.5f;

			Vector3 lightPos = transform.Translation + Vector3.Up * 3.0f;
			lightPos.X -= 2.0f;
			m_PointLight.transform.Translation = lightPos;
			m_PointLight.GetComponent<LightSource>().Specular = new Vector3((float)Math.Cos(delta) * 0.5f);

			ProcessMovement(delta);
			ProcessRotation(delta);
			//ProcessCameraRotation(delta);

			transform.Translate(Velocity);
			transform.Rotate(Rotation);
			//Velocity *= 0.5f;
			Rotation *= 0.5f;
		}

		void ProcessMovement(float delta)
		{
			float axisLeftTrigger = Input.GetGamepadAxis(Gamepad.AxisLeftTrigger);
			if (axisLeftTrigger > Deadzone)
				Velocity = -transform.Forward * MoveSpeed * delta * axisLeftTrigger;
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

		void ProcessCameraRotation(float delta)
		{
			// left right rotation
			float axisRightX = Input.GetGamepadAxis(Gamepad.AxisRightX);
			if (axisRightX > Deadzone || axisRightX < -Deadzone)
				m_Camera.transform.Rotate(m_Camera.transform.Up * RotationSpeed * delta * axisRightX);

			// up down rotation
			float axisRightY = Input.GetGamepadAxis(Gamepad.AxisRightY);
			if (axisRightY > Deadzone || axisRightY < -Deadzone)
				m_Camera.transform.Rotate(m_Camera.transform.Right * RotationSpeed * delta * axisRightY);
		}

		public override void OnGui()
		{
			if (!Scene.IsPaused())
				return;

			Gui.Begin("Pause Menu", new Vector2(), new Vector2());
			
			if (Gui.Button("Resume"))
				Scene.Resume();

			Gui.End();
		}
	}

}

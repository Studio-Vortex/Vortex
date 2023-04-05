using System;
using Vortex;

namespace Sandbox {

	public class MainMenu : Entity
	{
		public float Speed;
		public float ControllerDeadzone = 0.15f;
		public Vector3 Velocity;

		protected override void OnCreate()
		{
			// Called once before the first frame

			GetComponent<AudioSource>().Play();
		}

		protected override void OnUpdate(float delta)
		{
			// Called once every frame

			Velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W) || Input.GetGamepadAxis(GamepadAxis.LeftY) < -ControllerDeadzone)
				Velocity.Y = 1.0f * -Input.GetGamepadAxis(GamepadAxis.LeftY);
			else if (Input.IsKeyDown(KeyCode.S) || Input.GetGamepadAxis(GamepadAxis.LeftY) > ControllerDeadzone)
				Velocity.Y = -1.0f * Input.GetGamepadAxis(GamepadAxis.LeftY);

			if (Input.IsKeyDown(KeyCode.A) || Input.GetGamepadAxis(GamepadAxis.LeftX) < -ControllerDeadzone)
				Velocity.X = -1.0f * -Input.GetGamepadAxis(GamepadAxis.LeftX);
			else if (Input.IsKeyDown(KeyCode.D) || Input.GetGamepadAxis(GamepadAxis.LeftX) > ControllerDeadzone)
				Velocity.X = 1.0f * Input.GetGamepadAxis(GamepadAxis.LeftX);

			if (Input.IsGamepadButtonDown(GamepadButton.A))
				SceneManager.LoadScene("Real3D");

			transform.Translate(Velocity * Speed * delta);
		}

		protected override void OnGui()
		{
			Gui.Begin("Main Menu", Application.GetPosition(), Application.GetSize());
			Gui.Text("Use the left stick to move around");
			Gui.Text("Press A to load the next scene!");

			if (Scene.IsPaused())
			{
				if (Gui.Button("Resume the scene!"))
					Scene.Resume();
			}
			else
			{
				if (Gui.Button("Pause the scene!"))
					Scene.Pause();
			}

			Gui.Spacing();
			Gui.Text("Or click this button to load the Spawner Scene");
			Gui.Spacing();

			if (Gui.Button("Load Scene"))
				SceneManager.LoadScene("Spawner");

			Gui.End();
		}
	}

}

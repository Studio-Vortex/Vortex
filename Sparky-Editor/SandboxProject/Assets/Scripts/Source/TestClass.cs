using System;
using Sparky;

namespace Sandbox {

	public class TestClass : Entity
	{
		public float Speed;
		public float ControllerDeadzone = 0.15f;
		public Vector3 Velocity;

		public override void OnCreate()
		{
			// Called once before the first frame

			GetComponent<AudioSource>().Play();
		}

		public override void OnUpdate(float delta)
		{
			// Called once every frame

			Velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W) || Input.GetGamepadAxis(Gamepad.AxisLeftY) < -ControllerDeadzone)
				Velocity.Y = 1.0f * -Input.GetGamepadAxis(Gamepad.AxisLeftY);
			else if (Input.IsKeyDown(KeyCode.S) || Input.GetGamepadAxis(Gamepad.AxisLeftY) > ControllerDeadzone)
				Velocity.Y = -1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftY);

			if (Input.IsKeyDown(KeyCode.A) || Input.GetGamepadAxis(Gamepad.AxisLeftX) < -ControllerDeadzone)
				Velocity.X = -1.0f * -Input.GetGamepadAxis(Gamepad.AxisLeftX);
			else if (Input.IsKeyDown(KeyCode.D) || Input.GetGamepadAxis(Gamepad.AxisLeftX) > ControllerDeadzone)
				Velocity.X = 1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftX);

			if (Input.IsGamepadButtonDown(Gamepad.ButtonA))
				SceneManager.LoadScene("Real3D");

			transform.Translate(Velocity * Speed * delta);
		}

		public override void OnGui()
		{
			Vector2 windowPos = Window.GetPosition();
			windowPos.Y += 25.0f;
			Gui.Begin("Tutorial", windowPos, new Vector2(325.0f, 150.0f));
			Gui.Text("Welcome to Sparky!");
			Gui.Separator();
			Gui.Spacing();
			Gui.Text("Use the left stick to move around");
			Gui.Text("Press A to load the next scene!");

			Gui.Spacing();
			Gui.Text("Or click this button to load the Spawner Scene");
			Gui.Spacing();

			if (Gui.Button("Load Scene"))
				SceneManager.LoadScene("Spawner");

			Gui.End();
		}
	}

}

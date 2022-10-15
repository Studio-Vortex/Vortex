using System;
using Sparky;

namespace Sandbox {

	public class TestClass : Entity
	{
		public float Speed;
		public Vector3 Velocity;

		public override void OnCreate()
		{
			// Called once before the first frame
			Debug.Error("Hello from the game code!");
		}

		public override void OnUpdate(float delta)
		{
			// Called once every frame

			Velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				Velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				Velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				Velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				Velocity.X = 1.0f;

			transform.Translate(Velocity * Speed * delta);
		}

		public override void OnGui()
		{
			Gui.Begin("Window");

			if (Gui.Button("Play Music"))
				GetComponent<AudioSource>().Play();

			Gui.End();
		}
	}

}

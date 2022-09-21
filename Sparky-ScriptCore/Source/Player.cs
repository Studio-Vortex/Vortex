using System;

using Sparky;

namespace Sandbox {

	public class Player : Entity
	{
		void OnCreate()
		{
			Console.WriteLine($"Player.OnCreate - ID = {ID}");
		}

		void OnUpdate(float delta)
		{
			Console.WriteLine($"Player.OnUpdate - {delta}");

			Vector3 velocity = Vector3.Zero;
			float speed = 3.0f;

			if (Input.IsKeyDown(KeyCode.W))
				velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity.X = 1.0f;

			if (Translation.Y < -5.0f)
				Translation = Vector3.Zero;

			velocity *= speed * delta;

			Translation += velocity;
		}
	}

}

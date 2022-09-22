using System;

using Sparky;

namespace Sandbox {

	public class Camera : Entity
	{
		void OnUpdate(float delta)
		{
			Vector3 velocity = Vector3.Zero;
			float speed = 5.0f;

			if (Input.IsKeyDown(KeyCode.W))
				velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity.X = 1.0f;

			velocity *= speed * delta;
			Transform.Translation += velocity;
		}
	}

}

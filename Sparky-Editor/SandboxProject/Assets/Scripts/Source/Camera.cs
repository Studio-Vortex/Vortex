using System;

using Sparky;

namespace Sandbox {

	public class Camera : Entity
	{
		public Vector3 Velocity = Vector3.Zero;
		public float Speed;

		public void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Up))
				Velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.Down))
				Velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.Left))
				Velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.Right))
				Velocity.X = 1.0f;

			Velocity *= Speed * delta;
			transform.Translation += Velocity;
		}
	}

}

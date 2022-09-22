using System;

using Sparky;

namespace Sandbox {

	public class Camera : Entity
	{
		public Vector3 Velocity = Vector3.Zero;
		public float Speed;

		void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
				Velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				Velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				Velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				Velocity.X = 1.0f;

			Velocity *= Speed * delta;
			transform.Translation += Velocity;
		}
	}

}

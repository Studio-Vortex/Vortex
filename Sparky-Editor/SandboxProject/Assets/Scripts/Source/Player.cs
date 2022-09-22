using System;

using Sparky;

namespace Sandbox {

	public class Player : Entity
	{
		public Vector3 Velocity = Vector3.Zero;
		public float Speed;
		public float Time;
		public RigidBody2D rb;

		void OnCreate()
		{
			Time = 0.0f;
			rb = GetComponent<RigidBody2D>();
		}

		void OnUpdate(float delta)
		{
			Time += delta;

			if (Input.IsKeyDown(KeyCode.W))
				Velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				Velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				Velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				Velocity.X = 1.0f;

			if (transform.Translation.Y < -5.0f)
				transform.Translation = Vector3.Zero;

			Velocity *= Speed * delta;
			rb.ApplyLinearImpulse(Velocity.XY, true);
		}
	}

}

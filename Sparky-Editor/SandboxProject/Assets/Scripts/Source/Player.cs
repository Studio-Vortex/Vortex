using System;

using Sparky;

namespace Sandbox {

	public class Player : Entity
	{
		public Vector3 Velocity = Vector3.Zero;
		public float Speed;
		private RigidBody2D m_RigidBody;

		void OnCreate()
		{
			Console.WriteLine($"Player.OnCreate - {ID}");

			m_RigidBody = GetComponent<RigidBody2D>();
		}

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

			if (Transform.Translation.Y < -5.0f)
				Transform.Translation = Vector3.Zero;

			Velocity *= Speed;
			m_RigidBody.ApplyLinearImpulse(Velocity.XY, true);
		}
	}

}

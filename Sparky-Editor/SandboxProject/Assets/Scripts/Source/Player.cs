using System;

using Sparky;

namespace Sandbox {

	public class Player : Entity
	{
		private RigidBody2D m_RigidBody;

		void OnCreate()
		{
			Console.WriteLine($"Player.OnCreate - {ID}");

			m_RigidBody = GetComponent<RigidBody2D>();
		}

		void OnUpdate(float delta)
		{
			Vector3 velocity = Vector3.Zero;
			float speed = 0.25f;

			if (Input.IsKeyDown(KeyCode.W))
				velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity.X = 1.0f;

			if (Transform.Translation.Y < -5.0f)
				Transform.Translation = Vector3.Zero;

			velocity *= speed;
			m_RigidBody.ApplyLinearImpulse(velocity.XY, true);
		}
	}

}

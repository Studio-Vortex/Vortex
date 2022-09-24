using System;

using Sparky;

namespace Sandbox {

	public class Player : Entity
	{
		public Vector3 Velocity;
		public float Speed;
		public float Time;
		public RigidBody2D rb;
		private Camera2D m_Camera;

		public void OnCreate()
		{
			Time = 0.0f;
			rb = GetComponent<RigidBody2D>();
			m_Camera = FindEntityByName("Camera").As<Camera2D>();
		}

		public void OnUpdate(float delta)
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

			if (Input.IsKeyDown(KeyCode.Q))
				m_Camera.DistanceToPlayer += 2.0f * delta;
			if (Input.IsKeyDown(KeyCode.E))
				m_Camera.DistanceToPlayer -= 2.0f * delta;

			Velocity *= Speed * delta;
			rb.ApplyLinearImpulse(Velocity.XY, true);
		}
	}

}

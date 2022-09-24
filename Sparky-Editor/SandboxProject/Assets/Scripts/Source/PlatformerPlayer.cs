using System;
using Sparky;

namespace Sandbox {

	public class PlatformerPlayer : Entity
	{
		public float Speed;
		private Vector2 m_Velocity;
		private RigidBody2D m_Rigidbody;
		private CircleCollider2D m_CircleCollider;

		public void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
		}

		public void OnUpdate(float delta)
		{
			m_Velocity = Vector2.Zero;

			if (Input.IsKeyDown(KeyCode.A))
				m_Velocity.X = -1.0f;
			if (Input.IsKeyDown(KeyCode.D))
				m_Velocity.X = 1.0f;

			if (Input.IsKeyDown(KeyCode.P))
				m_CircleCollider.Restitution += 0.1f * delta;
			if (Input.IsKeyDown(KeyCode.L))
				m_CircleCollider.Restitution -= 0.1f * delta;

			m_Velocity *= Speed * 5.0f * delta;

			m_Rigidbody.ApplyForce(m_Velocity, true);
		}

		public void OnDestroy()
		{

		}
	}

}

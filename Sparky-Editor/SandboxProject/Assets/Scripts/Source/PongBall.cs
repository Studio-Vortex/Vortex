using System;
using Sparky;

namespace Sandbox {

	public class PongBall : Entity
	{
		public float Speed;
		private RigidBody2D m_Rigidbody;
		private Entity m_LeftPaddle;
		private Entity m_RightPaddle;
		private Transform m_LeftTransform;
		private Transform m_RightTransform;

		public void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();

			m_LeftPaddle = FindEntityByName("Left Paddle");
			m_LeftTransform = m_LeftPaddle.GetComponent<Transform>();
			m_RightPaddle = FindEntityByName("Right Paddle");
			m_RightTransform = m_RightPaddle.GetComponent<Transform>();
		}

		public void OnUpdate(float delta)
		{
			ApplyImpusle(delta);
		}

		private void ApplyImpusle(float delta = 0.0f)
		{
			Vector2 velocity;

			m_Rigidbody.ApplyLinearImpulse(velocity * Speed * delta, true);
		}
	}

}

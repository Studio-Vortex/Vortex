using System;
using Sparky;

namespace Sandbox {

	public class PongBall : Entity
	{
		public float Speed;
		public float MaxSpeed;
		private RigidBody2D m_Rigidbody;
		private float m_XLastFrame;

		public override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
		}

		public override void OnUpdate(float delta)
		{
			ApplyForce(delta);

			m_XLastFrame = transform.Translation.X;
		}

		private void ApplyForce(float delta)
		{
			float y = 0.0f;

			if (m_XLastFrame > transform.Translation.X)
				Speed = -Math.Abs(Speed);
			if (m_XLastFrame < transform.Translation.X)
				Speed = Math.Abs(Speed);

			if (transform.Translation.Y >= 4.5f)
			{
				Debug.Log("Ball hit roof");
				y = -Math.Abs(Speed);
			}
			if (transform.Translation.Y <= -4.5f)
			{
				Debug.Log("Ball hit floor");
				y = Math.Abs(Speed);
			}

			if (Speed > MaxSpeed)
				Speed = MaxSpeed;

			m_Rigidbody.ApplyForce(new Vector2(Speed * delta, y * delta), true);
		}
	}

}

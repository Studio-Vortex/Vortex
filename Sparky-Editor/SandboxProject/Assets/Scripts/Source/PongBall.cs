using System;
using Sparky;

namespace Sandbox {

	public class PongBall : Entity
	{
		public float Speed;
		public float MaxSpeed;
		public Vector2 Velocity;

		public bool ShowRaycasts;

		private RigidBody2D m_Rigidbody;
		private CircleCollider2D m_CircleCollider;

		public override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
			m_CircleCollider = GetComponent<CircleCollider2D>();
			m_Rigidbody.ApplyForce(new Vector2(MaxSpeed, -1.0f), true);
		}

		public override void OnUpdate(float delta)
		{
			float currentX = transform.Translation.X;
			float currentY = transform.Translation.Y;
			float circleRadius = m_CircleCollider.Radius;

			float upPoint = currentY + circleRadius;
			bool hitUp = Physics2D.Raycast(transform.Translation.XY, new Vector2(currentX, upPoint), ShowRaycasts);

			if (hitUp)
				Velocity.Y = -1.0f;

			float downPoint = currentY - circleRadius;
			bool hitDown = Physics2D.Raycast(transform.Translation.XY, new Vector2(currentX, downPoint), ShowRaycasts);

			if (hitDown)
				Velocity.Y = 1.0f;

			float leftPoint = currentX - circleRadius;
			bool hitLeft = Physics2D.Raycast(transform.Translation.XY, new Vector2(leftPoint, currentY), ShowRaycasts);

			if (hitLeft)
				Velocity.X = 1.0f;

			float rightPoint = currentX + circleRadius;
			bool hitRight = Physics2D.Raycast(transform.Translation.XY, new Vector2(rightPoint, currentY), ShowRaycasts);

			if (hitRight)
				Velocity.Y = -1.0f;

			Speed = Math.Min(Speed, MaxSpeed);

			Velocity *= Speed * delta;
			m_Rigidbody.ApplyForce(Velocity, true);
		}
	}

}

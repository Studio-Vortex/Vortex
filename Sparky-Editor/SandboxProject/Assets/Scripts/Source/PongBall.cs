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
			Physics2D.Raycast(transform.Translation.XY, new Vector2(currentX, upPoint), out RayCastHit2D hit, ShowRaycasts);

			if (hit.Hit)
				Velocity.Y = -1.0f;

			float downPoint = currentY - circleRadius;
			Physics2D.Raycast(transform.Translation.XY, new Vector2(currentX, downPoint), out RayCastHit2D hit1, ShowRaycasts);

			if (hit1.Hit)
				Velocity.Y = 1.0f;

			float leftPoint = currentX - circleRadius;
			Physics2D.Raycast(transform.Translation.XY, new Vector2(leftPoint, currentY), out RayCastHit2D hit2, ShowRaycasts);

			if (hit2.Hit)
				Velocity.X = 1.0f;

			float rightPoint = currentX + circleRadius;
			Physics2D.Raycast(transform.Translation.XY, new Vector2(rightPoint, currentY), out RayCastHit2D hit3, ShowRaycasts);

			if (hit3.Hit)
				Velocity.Y = -1.0f;

			Speed += delta;

			Speed = Math.Min(Speed, MaxSpeed);

			Velocity *= Speed * delta;
			m_Rigidbody.ApplyForce(Velocity, true);
		}
	}

}

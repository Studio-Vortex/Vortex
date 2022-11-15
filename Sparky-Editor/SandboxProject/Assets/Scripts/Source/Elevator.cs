using System;
using Sparky;

namespace Sandbox {

	public class Elevator : Entity
	{
		public float Speed;
		public float StopPoint;
		public bool DestinationReached;
		public float StartY;
		public bool ShowRaycast;
		private BoxCollider2D m_BoxCollider;

		protected override void OnCreate()
		{
			m_BoxCollider = GetComponent<BoxCollider2D>();
			DestinationReached = false;
			StartY = transform.Translation.Y;
		}

		protected override void OnUpdate(float delta)
		{
			Vector2 landingPoint = transform.Translation.XY;
			landingPoint.Y += m_BoxCollider.Size.Y + m_BoxCollider.Offset.Y + 0.1f;

			Entity player = Physics2D.Raycast(transform.Translation.XY, landingPoint, out RayCastHit2D hit, ShowRaycast);

			float moveSpeed = Speed * delta;

			if (hit.Hit && hit.Tag == "Player" && !DestinationReached)
			{
				transform.Translate(0.0f, moveSpeed, 0.0f);

				if (transform.Translation.Y > StopPoint)
					DestinationReached = true;
			}
			else if (!hit.Hit && DestinationReached)
			{
				transform.Translate(0.0f, -moveSpeed, 0.0f);

				if (transform.Translation.Y < StartY)
					DestinationReached = false;
			}
		}
	}

}

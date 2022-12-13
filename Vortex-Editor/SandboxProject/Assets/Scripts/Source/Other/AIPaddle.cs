using System;
using Vortex;

namespace Sandbox {

	class AIPaddle : Entity
	{
		public float MaxSpeed;
		private Entity m_Ball;
		private Transform m_BallTransform;
		private Vector3 m_BallPosition;
		private Vector3 m_Velocity;

		protected override void OnCreate()
		{
			m_Ball = FindEntityByName("Ball");
			m_BallTransform = m_Ball.GetComponent<Transform>();
		}

		protected override void OnUpdate(float delta)
		{
			m_BallPosition = m_BallTransform.Translation;

			m_Velocity.Y = m_BallPosition.Y;

			if (transform.Translation.Y > 3.25f)
				transform.Translation = new Vector3(transform.Translation.X, 3.25f, transform.Translation.Z);
			if (transform.Translation.Y < -3.25f)
				transform.Translation = new Vector3(transform.Translation.X, -3.25f, transform.Translation.Z);

			if (m_Velocity.Y > MaxSpeed)
				m_Velocity.Y = MaxSpeed;
			if (m_Velocity.Y < -MaxSpeed)
				m_Velocity.Y = -MaxSpeed;

			transform.Translation += m_Velocity * delta;
		}
	}

}

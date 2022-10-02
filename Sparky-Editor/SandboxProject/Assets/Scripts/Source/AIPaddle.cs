using System;
using Sparky;

namespace Sandbox {

	class AIPaddle : Entity
	{
		public float MaxSpeed;
		private Entity m_Ball;
		private Transform m_BallTransform;
		private Vector3 m_BallPosition;

		public void OnCreate()
		{
			m_Ball = FindEntityByName("Ball");
			m_BallTransform = m_Ball.GetComponent<Transform>();
		}

		public void OnUpdate(float delta)
		{
			m_BallPosition = m_BallTransform.Translation;

			transform.Translation = new Vector3(transform.Translation.X, m_BallPosition.Y, transform.Translation.Z);

			if (transform.Translation.Y > 3.25f)
				transform.Translation = new Vector3(transform.Translation.X, 3.25f, transform.Translation.Z);
			if (transform.Translation.Y < -3.25f)
				transform.Translation = new Vector3(transform.Translation.X, -3.25f, transform.Translation.Z);
		}
	}

}

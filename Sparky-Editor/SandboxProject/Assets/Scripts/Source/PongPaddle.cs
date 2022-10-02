using System;
using Sparky;

namespace Sandbox {

	public class PongPaddle : Entity
	{
		public float Speed;
		public bool LeftPaddle;
		private Vector2 m_Velocity;

		public void OnUpdate(float delta)
		{
			if (LeftPaddle)
			{
				if (Input.IsKeyDown(KeyCode.W))
					m_Velocity.Y = 1.0f;
				if (Input.IsKeyDown(KeyCode.S))
					m_Velocity.Y = -1.0f;
			}
			else
			{
				if (Input.IsKeyDown(KeyCode.I))
					m_Velocity.Y = 1.0f;
				if (Input.IsKeyDown(KeyCode.K))
					m_Velocity.Y = -1.0f;
			}

			m_Velocity *= Speed * delta;

			transform.Translation += new Vector3(m_Velocity, 0.0f);
		}
	}

}

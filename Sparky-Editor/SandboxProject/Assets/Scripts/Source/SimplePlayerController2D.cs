using System;
using Sparky;

namespace Sandbox {

	public class SimplePlayerController2D : Entity
	{
		public float Speed;
		public Vector3 StartPos;
		private Vector3 m_Velocity;

		public override void OnCreate()
		{
			StartPos = transform.Translation;
		}

		public override void OnUpdate(float delta)
		{
			m_Velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.A))
				m_Velocity.X = -Speed;
			else if (Input.IsKeyDown(KeyCode.D))
				m_Velocity.X = Speed;

			transform.Translation += m_Velocity * delta;

			if (Input.IsKeyDown(KeyCode.R))
				RestartGame();
		}

		public void RestartGame()
		{
			transform.Translation = StartPos;
			m_Velocity = Vector3.Zero;
		}

		public override void OnDestroy()
		{
			Debug.Info("Player Killed");
		}
	}

}

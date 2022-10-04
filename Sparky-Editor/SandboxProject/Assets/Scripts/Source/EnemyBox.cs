using System;
using Sparky;

namespace Sandbox {

	public class EnemyBox : Entity
	{
		public float Speed;
		public float Bounds;
		private Entity m_TargetPlayer;
		private Vector3 m_Velocity;
		private bool m_IsMovingUp;

		public override void OnCreate()
		{
			m_TargetPlayer = FindEntityByName("Player");
			m_IsMovingUp = true;
		}

		public override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Space))
			{
				RemoveComponent<SpriteRenderer>();
			}

			if (m_IsMovingUp)
			{
				m_Velocity.Y = 1.0f;
				if (transform.Translation.Y >= Bounds)
					m_IsMovingUp = false;
			}
			else
			{
				m_Velocity.Y = -1.0f;
				if (transform.Translation.Y <= -Bounds)
					m_IsMovingUp = true;
			}

			m_Velocity *= Speed * delta;

			transform.Translation += m_Velocity;
		}
	}

}

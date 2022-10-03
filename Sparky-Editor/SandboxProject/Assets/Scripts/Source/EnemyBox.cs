using System;
using Sparky;

namespace Sandbox {

	public class EnemyBox : Entity
	{
		public float Speed;
		private Entity m_TargetPlayer;
		private Vector3 m_Velocity;
		private float m_YPosLastFrame;

		public override void OnCreate()
		{
			m_TargetPlayer = FindEntityByName("Player");
		}

		public override void OnUpdate(float delta)
		{
			if (m_YPosLastFrame > transform.Translation.Y)
				m_Velocity.Y = -1.0f * Speed;
			if (m_YPosLastFrame < transform.Translation.Y)
				m_Velocity.Y = 1.0f * Speed;

			m_YPosLastFrame = transform.Translation.Y;
		}
	}

}

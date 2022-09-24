using System;

using Sparky;

namespace Sandbox {

	public class Enemy : Entity
	{
		public float Speed;
		private Entity m_Target;
		private Vector3 m_Velocity;
		private Transform m_TargetTransform;

		public void OnCreate()
		{
			m_Target = FindEntityByName("Player");
			m_TargetTransform = m_Target.GetComponent<Transform>();
		}

		public void OnUpdate(float delta)
		{
			Vector3 targetPosition = m_TargetTransform.Translation;

			if (transform.Translation.X < targetPosition.X)
				m_Velocity.X = 1.0f;

			if (transform.Translation.X > targetPosition.X)
				m_Velocity.X = -1.0f;

			if (transform.Translation == targetPosition)
				m_Target.Destroy();

			m_Velocity *= Speed * delta;

			transform.Translation += m_Velocity;
		}

		public void OnDestroy()
		{
			
		}
	}

}

using System;
using Sparky;

namespace Sandbox {

	public class Enemy : Entity
	{
		public float Speed;
		private Entity m_Target;
		private Vector3 m_Velocity;
		private Transform m_TargetTransform;
		private RigidBody2D m_RigidBody;

		public override void OnCreate()
		{
			m_Target = FindEntityByName("Player");
			m_TargetTransform = m_Target.GetComponent<Transform>();
			m_RigidBody = GetComponent<RigidBody2D>();
		}

		public override void OnUpdate(float delta)
		{
			Vector3 targetPosition = m_TargetTransform.Translation;

			if (transform.Translation.X < targetPosition.X)
				m_Velocity.X = 1.0f;

			if (transform.Translation.X > targetPosition.X)
				m_Velocity.X = -1.0f;

			if (transform.Translation == targetPosition)
				m_Target.Destroy();

			m_Velocity *= Speed * delta;
			m_RigidBody.ApplyForce(m_Velocity.XY, true);
		}

		public override void OnDestroy()
		{
			Debug.Log("Enemy Killed");
		}
	}

}

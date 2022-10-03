using System;
using Sparky;

namespace Sandbox {

	public class SimplePlayerController2D : Entity
	{
		public float Speed;
		private RigidBody2D m_Rigidbody;
		private Vector3 m_Velocity;

		public override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
		}

		public override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.A))
				m_Velocity.X = -Speed;
			else if (Input.IsKeyDown(KeyCode.D))
				m_Velocity.X = Speed;

			m_Rigidbody.ApplyForce(m_Velocity.XY * delta, true);
		}

		public override void OnDestroy()
		{
			Debug.Info("Player Killed");
		}
	}

}

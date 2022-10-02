using System;
using Sparky;

namespace Sandbox {

	public class PongBall : Entity
	{
		public float Speed;
		private RigidBody2D m_Rigidbody;

		public void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
			m_Rigidbody.ApplyForce(new Vector2(25, 0), true);
		}

		public void OnUpdate(float delta)
		{
			
		}
	}

}

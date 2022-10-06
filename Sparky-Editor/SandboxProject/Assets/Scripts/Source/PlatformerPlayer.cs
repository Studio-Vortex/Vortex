using System;
using Sparky;

namespace Sandbox {

	public class PlatformerPlayer : Entity
	{
		public float Speed;
		public float JumpForce;
		public bool IsGrounded;
		public float Friction;
		private Vector3 m_Velocity;
		private RigidBody2D m_Rigidbody;

		public override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
			Debug.Info("Sandbox.PlatformerPlayer::OnCreate()");
		}

		public override void OnUpdate(float delta)
		{
			IsGrounded = false;

			if (Input.IsKeyDown(KeyCode.A))
				m_Velocity.X = -1.0f;
			if (Input.IsKeyDown(KeyCode.D))
				m_Velocity.X = 1.0f;

			if (transform.Translation.Y == 0.0f)
				IsGrounded = true;

			if (Input.IsKeyDown(KeyCode.Space) && IsGrounded)
				m_Velocity.Y = 1.0f;

			//if (!IsGrounded)
				//m_Velocity.Y /= Friction;

			m_Velocity *= Speed * delta;
			m_Rigidbody.ApplyLinearImpulse(m_Velocity.XY, true);
		}

		public override void OnDestroy()
		{
			Debug.Info("Sandbox.PlatformerPlayer::OnDestroy()");
		}
	}

}

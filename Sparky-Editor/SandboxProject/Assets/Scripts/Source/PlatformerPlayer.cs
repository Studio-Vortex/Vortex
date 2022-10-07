using System;
using Sparky;

namespace Sandbox {

	public class PlatformerPlayer : Entity
	{
		public float AnimationWaitTime = 1.0f;
		public float Friction = 2.0f;
		public float JumpForce;
		public float PlayerResetYAxis = -50.0f;
		public float Speed;
		public float YPosLastFrame;
		public float WaitTime;
		public Vector3 StartPosition;
		public Vector3 Velocity;

		public bool DisableRunAnimation;
		public bool IsGrounded;

		private const string m_NormalTextureString = "Assets/Textures/game/Platformer/Characters/character_0000.png";
		private const string m_JumpTextureString = "Assets/Textures/game/Platformer/Characters/character_0001.png";
		private RigidBody2D m_Rigidbody;
		private SpriteRenderer m_Sprite;
		private bool m_AnimationReady;

		public override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
			m_Sprite = GetComponent<SpriteRenderer>();
			StartPosition = transform.Translation;
		}

		public override void OnUpdate(float delta)
		{
			IsGrounded = false;

			if (WaitTime <= 0.0f)
			{
				WaitTime = AnimationWaitTime;
				m_AnimationReady = true;
			}

			WaitTime -= delta;

			if (transform.Translation.Y <= PlayerResetYAxis)
			{
				Velocity = Vector3.Zero;
				transform.Translation = StartPosition;
			}

			if (Input.IsKeyDown(KeyCode.A))
			{
				transform.Rotation = new Vector3(0.0f, 0.0f, 0.0f);
				Velocity.X = -1.0f;

				PlayRunningAnimation();
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				transform.Rotation = new Vector3(0.0f, 180.0f, 0.0f);
				Velocity.X = 1.0f;

				PlayRunningAnimation();
			}

			if (YPosLastFrame == transform.Translation.Y)
				IsGrounded = true;

			if (!IsGrounded)
				m_Sprite.Texture = m_JumpTextureString;
			else
				m_Sprite.Texture = m_NormalTextureString;

			if (Input.IsKeyDown(KeyCode.Space) && IsGrounded)
				Velocity.Y = 1.0f;

			if (!IsGrounded)
				Velocity.Y /= Friction;

			YPosLastFrame = transform.Translation.Y;

			Velocity.X *= Speed * delta;
			Velocity.Y *= JumpForce * delta;
			m_Rigidbody.ApplyLinearImpulse(Velocity.XY, true);
		}

		private void PlayRunningAnimation()
		{
			if (!IsGrounded && !m_AnimationReady && !DisableRunAnimation)
				return;

			if (m_Sprite.Texture == m_NormalTextureString)
				m_Sprite.Texture = m_JumpTextureString;
			else
				m_Sprite.Texture = m_NormalTextureString;

			m_AnimationReady = false;
		}

		public override void OnDestroy()
		{
			Debug.Info("Sandbox.PlatformerPlayer::OnDestroy()");
		}
	}

}

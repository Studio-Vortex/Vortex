using System;
using Sparky;

namespace Sandbox {

	public class PlatformerPlayer : Entity
	{
		public float Speed;
		public float JumpForce;
		public float Friction;
		public float YPosLastFrame;
		public float PlayerResetYAxis = -5.0f;
		public float Time;
		public float AnimationWaitTime;
		private float m_WaitTime;
		public Vector3 Velocity;
		public Vector3 StartPosition;

		public bool IsGrounded;

		private Camera2D m_CameraEntity;
		private RigidBody2D m_Rigidbody;
		private SpriteRenderer m_Sprite;
		private bool m_AnimationReady;

		public override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
			m_Sprite = GetComponent<SpriteRenderer>();
			m_CameraEntity = FindEntityByName("Camera").As<Camera2D>();

			Debug.Info("Sandbox.PlatformerPlayer::OnCreate()");
			StartPosition = transform.Translation;
		}

		public override void OnUpdate(float delta)
		{
			Camera camera = m_CameraEntity.GetComponent<Camera>();
			camera.FixedAspectRatio = true;

			IsGrounded = false;

			Time += delta;
			if (m_WaitTime == 0.0f)
			{
				m_WaitTime = AnimationWaitTime;
				m_AnimationReady = true;
			}

			if (transform.Translation.Y <= PlayerResetYAxis)
			{
				Velocity = Vector3.Zero;
				ResetPlayerPosition();
			}

			if (Input.IsKeyDown(KeyCode.A))
			{
				transform.Rotation = new Vector3(0.0f, 0.0f, 0.0f);
				Velocity.X = -1.0f;
				PlayRunningAnimation();
			}
			if (Input.IsKeyDown(KeyCode.D))
			{
				transform.Rotation = new Vector3(0.0f, 180.0f, 0.0f);
				Velocity.X = 1.0f;
				PlayRunningAnimation();
			}

			if (YPosLastFrame == transform.Translation.Y)
				IsGrounded = true;

			if (YPosLastFrame < transform.Translation.Y)
				m_Sprite.Texture = "Assets/Textures/game/Platformer/Characters/character_0001.png";
			else
				m_Sprite.Texture = "Assets/Textures/game/Platformer/Characters/character_0000.png";

			if (Input.IsKeyDown(KeyCode.Space) && IsGrounded)
				Velocity.Y = 1.0f;

			if (!IsGrounded)
				Velocity.Y /= Friction;

			YPosLastFrame = transform.Translation.Y;

			Velocity.X *= Speed * delta;
			Velocity.Y *= JumpForce * delta;
			m_Rigidbody.ApplyLinearImpulse(Velocity.XY, true);
		}

		private void ResetPlayerPosition()
		{
			transform.Translation = StartPosition;
		}

		private void PlayRunningAnimation()
		{
			if (!m_AnimationReady)
				return;

			if (m_Sprite.Texture == "Assets/Textures/game/Platformer/Characters/character_0000.png")
				m_Sprite.Texture = "Assets/Textures/game/Platformer/Characters/character_0001.png";
			else
				m_Sprite.Texture = "Assets/Textures/game/Platformer/Characters/character_0000.png";

			m_AnimationReady = false;
		}

		public override void OnDestroy()
		{
			Debug.Info("Sandbox.PlatformerPlayer::OnDestroy()");
		}
	}

}

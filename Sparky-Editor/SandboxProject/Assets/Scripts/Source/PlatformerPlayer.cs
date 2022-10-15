using System;
using Sparky;

namespace Sandbox {

	public class PlatformerPlayer : Entity
	{
		public float AnimationWaitTime = 1.0f;
		public float JumpForce;
		public float PlayerResetYAxis = -50.0f;
		public float FinishPoint;
		public float HorizontalFriction = 2.0f;
		public float Speed;
		public float WaitTime;
		public Vector3 StartPosition;
		public Vector3 Velocity;

		public bool IsGrounded;
		public bool AnimationReady;
		public bool IsRunning;
		public bool ShowRaycast;
		public bool GameOver;

		private const string m_NormalTextureString = "Assets/Textures/game/Platformer/Characters/character_0000.png";
		private const string m_JumpTextureString = "Assets/Textures/game/Platformer/Characters/character_0001.png";
		
		private RigidBody2D m_Rigidbody;
		private BoxCollider2D m_BoxCollider;
		private SpriteRenderer m_Sprite;
		private AudioSource m_JumpSound;

		private Camera2D m_CameraEntity;

		public override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
			m_BoxCollider = GetComponent<BoxCollider2D>();
			m_Sprite = GetComponent<SpriteRenderer>();
			m_JumpSound = GetComponent<AudioSource>();

			StartPosition = transform.Translation;

			m_CameraEntity = FindEntityByName("Camera").As<Camera2D>();
		}

		public override void OnUpdate(float delta)
		{
			Vector2 playerFootPoint = new Vector2(transform.Translation.X, (transform.Translation.Y - m_BoxCollider.Size.Y) + m_BoxCollider.Offset.Y);
			Vector2 groundPoint = playerFootPoint;
			groundPoint.Y -= 0.1f;

			Entity entity = Physics2D.Raycast(transform.Translation.XY, groundPoint, out RayCastHit2D hit, ShowRaycast);
			IsGrounded = hit.Hit;

			if (hit.Hit && hit.Tag == "Grass Collider")
			{
				m_Sprite.Color = new Vector4(0.8f, 0.4f, 0.4f, 1.0f);
				entity.Destroy(scriptInstance: false);
			}

			if (transform.Translation.Y <= PlayerResetYAxis)
			{
				Velocity = Vector3.Zero;
				transform.Translation = StartPosition;
				m_Sprite.Color = new Vector4(1.0f);
			}

			if (Input.IsKeyDown(KeyCode.Escape))
				Game.Shutdown();

			if (Input.IsKeyDown(KeyCode.A))
			{
				IsRunning = true;
				transform.Rotation = new Vector3(0.0f, 0.0f, 0.0f);
				Velocity.X = -1.0f;

				PlayRunningAnimation(delta);
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				IsRunning = true;
				transform.Rotation = new Vector3(0.0f, 180.0f, 0.0f);
				Velocity.X = 1.0f;

				PlayRunningAnimation(delta);
			}
			else
				IsRunning = false;

			if (transform.Translation.X >= FinishPoint)
				GameOver = true;

			if (GameOver && m_CameraEntity.DistanceToPlayer > 5.0f)
				m_CameraEntity.DistanceToPlayer -= 1.0f * delta;

			if (Input.IsKeyDown(KeyCode.Space) && IsGrounded)
			{
				m_JumpSound.Play();
				Velocity.Y = 1.0f;
			}

			if (!IsGrounded)
			{
				m_Sprite.Texture = m_JumpTextureString;
				Velocity.X /= HorizontalFriction;
			}
			else
				m_Sprite.Texture = m_NormalTextureString;

			Velocity.X *= Speed * delta;
			Velocity.Y *= JumpForce * delta;
			m_Rigidbody.ApplyLinearImpulse(Velocity.XY, true);
		}

		private void PlayRunningAnimation(float delta)
		{
			if (IsGrounded)
				WaitTime -= delta;

			if (WaitTime <= 0.0f)
			{
				WaitTime = AnimationWaitTime;
				AnimationReady = true;
			}

			if (!IsGrounded && !AnimationReady)
				return;

			if (m_Sprite.Texture == m_NormalTextureString)
			{
				m_Sprite.Texture = m_JumpTextureString;
				Debug.Log("setting texture");
			}
			else
				m_Sprite.Texture = m_NormalTextureString;

			AnimationReady = false;
		}

		public override void OnDestroy()
		{
			Debug.Info("Sandbox.PlatformerPlayer::OnDestroy()");
		}
	}

}

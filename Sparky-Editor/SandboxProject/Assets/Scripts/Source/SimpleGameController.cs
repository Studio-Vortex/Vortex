using Sparky;
using System;

namespace Sandbox {

	public class SimpleGameController : Entity
	{
		public float Speed;
		public float CameraDistance;
		public float FlashlightDistance;
		public bool IsGrounded;
		public bool IsMoving;
		public float JumpForce;

		private Entity m_CameraEntity;
		private Entity m_Flashlight;
		private RigidBody2D m_Rigidbody;
		private const float FRICTION = 2.0f;

		protected override void OnCreate()
		{
			m_Rigidbody = GetComponent<RigidBody2D>();
			m_CameraEntity = FindEntityByName("Camera");
			m_Flashlight = FindEntityByName("Flash Light");
		}

		protected override void OnUpdate(float delta)
		{
			IsGrounded = Grounded();
			SetEntityPositions();

			ProcessInput(delta);
		}

		void ProcessInput(float delta)
		{
			Vector2 velocity = Vector2.Zero;
			float speed = Speed * delta;

			if (Input.IsKeyDown(KeyCode.A))
			{
				velocity = Vector2.Left * speed;
				IsMoving = true;
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				velocity = Vector2.Right * speed;
				IsMoving = true;
			}
			else
				IsMoving = false;

			if (!IsMoving)
				velocity *= FRICTION;

			if (Input.IsKeyDown(KeyCode.Space) && IsGrounded)
				m_Rigidbody.ApplyForce(Vector2.Up * JumpForce, true);

			if (Input.IsKeyDown(KeyCode.Escape))
				Application.Shutdown();

			m_Rigidbody.ApplyLinearImpulse(velocity, true);
		}

		bool Grounded()
		{
			Vector2 groundPoint = transform.Translation.XY;
			groundPoint.Y -= 0.55f;
			Physics2D.Raycast(transform.Translation.XY, groundPoint, out RayCastHit2D hit);
			return hit.Hit;
		}

		void SetEntityPositions()
		{
			m_CameraEntity.transform.Translation = transform.Translation;
			m_CameraEntity.transform.Translate(0, 2, CameraDistance);
			m_Flashlight.transform.Translation = transform.Translation;
			m_Flashlight.transform.Translate(0, FlashlightDistance, 0);
		}
	}

}

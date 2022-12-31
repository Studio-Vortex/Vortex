using Vortex;

namespace Sandbox {

	public class SimpleGameController : Entity
	{
		public float Speed;
		public Vector3 CameraDistance;
		public float JumpForce;

		Entity m_CameraEntity;
		CharacterController m_Controller;
		AudioSource m_JumpSound;

		protected override void OnCreate()
		{
			m_Controller = GetComponent<CharacterController>();
			m_JumpSound = GetComponent<AudioSource>();
			m_CameraEntity = FindEntityByName("Camera");
		}

		protected override void OnUpdate(float delta)
		{
			SetEntityPositions();

			ProcessInput(delta);
		}

		protected override void OnCollisionBegin()
		{
			Debug.Info("Collision Occurred!");
		}

		void ProcessInput(float delta)
		{
			Vector2 velocity = Vector2.Zero;
			float speed = Speed * delta;

			if (Input.IsKeyDown(KeyCode.S))
			{
				velocity = Vector2.Left * speed;
			}
			else if (Input.IsKeyDown(KeyCode.W))
			{
				velocity = Vector2.Right * speed;
			}


			if (Input.IsKeyDown(KeyCode.Space) && m_Controller.IsGrounded)
			{
				m_Controller.Jump(JumpForce);
				if (!m_JumpSound.IsPlaying)
					m_JumpSound.Play();
			}

			m_Controller.Move(new Vector3(velocity, 0f));
		}

		void SetEntityPositions()
		{
			m_CameraEntity.transform.Translation = transform.Translation;
			m_CameraEntity.transform.Translate(CameraDistance);
		}
	}

}

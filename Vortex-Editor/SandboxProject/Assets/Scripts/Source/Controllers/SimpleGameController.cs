using Vortex;

namespace Sandbox {

	public class SimpleGameController : Entity
	{
		public float Speed;
		public float CameraDistance;
		public float FlashlightDistance;
		public float JumpForce;

		Entity m_CameraEntity;
		Entity m_Flashlight;
		CharacterController m_Controller;
		AudioSource m_AudioSource;

		protected override void OnCreate()
		{
			m_Controller = GetComponent<CharacterController>();
			m_AudioSource = GetComponent<AudioSource>();
			m_CameraEntity = FindEntityByName("Camera");
			m_Flashlight = FindEntityByName("Flash Light");
		}

		protected override void OnUpdate(float delta)
		{
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
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				velocity = Vector2.Right * speed;
			}


			if (Input.IsKeyDown(KeyCode.Space) && m_Controller.IsGrounded)
			{
				m_Controller.Jump(JumpForce);
				if (!m_AudioSource.IsPlaying)
					m_AudioSource.Play();
			}

			if (Input.IsKeyDown(KeyCode.Escape))
				Application.Quit();

			m_Controller.Move(new Vector3(velocity, 0f));
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

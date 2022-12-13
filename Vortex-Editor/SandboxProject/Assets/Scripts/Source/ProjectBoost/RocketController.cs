using Vortex;

namespace Sandbox {

	public class RocketController : Entity
	{
		public float mainThrust = 100f;
		public float rotationThrust = 1f;
		public static int level = 1;

		RigidBody2D rigidbody;
		AudioSource audioSource;
		ParticleEmitter particleEmitter;

		public float time = 0f;
		public float timeToWait = 3f;
		public bool loadingNextLevel = false;
		public bool reloadingLevel = false;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody2D>();
			audioSource = GetComponent<AudioSource>();
			particleEmitter = GetComponent<ParticleEmitter>();
		}

		protected override void OnUpdate(float delta)
		{
			ProcessThrust();
			ProcessRotation();

			if (Input.IsKeyDown(KeyCode.Escape))
			{
				Application.Quit();
			}

			ProcessHit();

			// Major hack because the engine doesn't have a Timer
			// we need to track time by our self
			if (loadingNextLevel)
			{
				time -= delta; // decrement timer

				if (time <= 0f) // check if timer is done
				{
					loadingNextLevel = false;

					// Load next level
					if (level == 1)
					{
						SceneManager.LoadScene("Boost_02");
						level++;
					}
					else if (level == 2)
					{
						SceneManager.LoadScene("Boost_03");
						level++;
					}
					else
					{
						SceneManager.LoadScene("Boost_01");
						level = 1;
					}
				}
			}
			else if (reloadingLevel)
			{
				time -= delta;

				if (time <= 0f)
				{
					reloadingLevel = false;

					if (level == 1)
						SceneManager.LoadScene("Boost_01");
					else if (level == 2)
						SceneManager.LoadScene("Boost_02");
					else
						SceneManager.LoadScene("Boost_03");
				}
			}
		}

		void ProcessThrust()
		{
			if (Input.IsKeyDown(KeyCode.Space))
			{
				rigidbody.ApplyForce(transform.Up.XY * mainThrust * Time.DeltaTime, true);
				audioSource.Play();
				particleEmitter.Start();
				particleEmitter.Velocity = -transform.Up;
			}
			else if (Input.IsKeyUp(KeyCode.Space))
			{
				audioSource.Stop();
				particleEmitter.Stop();
			}
		}

		void ProcessRotation()
		{
			if (Input.IsKeyDown(KeyCode.A))
			{
				ApplyRotation(-rotationThrust);
			}
			else if (Input.IsKeyDown(KeyCode.D))
			{
				ApplyRotation(rotationThrust);
			}
		}

		void ApplyRotation(float rotation)
		{
			rigidbody.FreezeRotation = true; // freezing rotation so we can manually rotate
			transform.Rotate(Vector3.Forward * rotation * Time.DeltaTime);
			rigidbody.FreezeRotation = false; // unfreeze so physics system can take over
		}

		void ProcessHit()
		{
			Vector2 pos = transform.Translation.XY;
			Vector2 groundPoint = pos;
			groundPoint.Y -= transform.Scale.Y / 1.8f;
			Entity entity = Physics2D.Raycast(pos, groundPoint, out RayCastHit2D downHit);

			if (downHit.Hit)
				ProcessEntity(entity);
		}

		void ProcessEntity(Entity entity)
		{
			switch (entity.Marker)
			{
				case "Start":
					
					break;
				case "Finish":
					LoadNextLevel(entity);
					break;
				case "Obstacle":
				default:
					ReloadLevel();
					break;
			}
		}

		void LoadNextLevel(Entity entity)
		{
			// TODO: Load next level here
			AudioSource audioSource = entity.GetComponent<AudioSource>();
			if (!audioSource.IsPlaying)
				audioSource.Play();

			if (!loadingNextLevel)
				time = timeToWait;

			loadingNextLevel = true;
		}

		void ReloadLevel()
		{
			Entity ground = FindEntityByName("Ground");
			AudioSource audioSource = ground.GetComponent<AudioSource>();

			if (!audioSource.IsPlaying)
				audioSource.Play();

			if (!reloadingLevel)
				time = timeToWait;

			reloadingLevel = true;
		}
	}

}

using Sparky;
using System;
using System.Threading;

namespace Sandbox {

	public class RocketController : Entity
	{
		public float mainThrust = 100f;
		public float rotationThrust = 1f;
		public static int level = 1;

		private RigidBody2D rigidbody;
		private AudioSource audioSource;
		private ParticleEmitter particleEmitter;

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
				Application.Shutdown();
			}

			ProcessHit();
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

		private static void LoadNextLevel(Entity entity)
		{
			// TODO: Load next level here
			AudioSource audioSource = entity.GetComponent<AudioSource>();
			if (!audioSource.IsPlaying)
				audioSource.Play();

			// Wait for x seconds
			Thread.Sleep(3000);

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

		void ReloadLevel()
		{
			Entity ground = FindEntityByName("Ground");
			AudioSource audioSource = ground.GetComponent<AudioSource>();

			if (!audioSource.IsPlaying)
				audioSource.Play();

			Thread.Sleep(3000);

			if (level == 1)
				SceneManager.LoadScene("Boost_01");
			else if (level == 2)
				SceneManager.LoadScene("Boost_02");
			else
				SceneManager.LoadScene("Boost_03");
		}
	}

}

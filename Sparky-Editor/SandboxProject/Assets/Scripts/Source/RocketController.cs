using Sparky;

namespace Sandbox {

	public class RocketController : Entity
	{
		public float mainThrust = 100f;
		public float rotationThrust = 1f;

		private RigidBody2D rigidbody;
		private AudioSource audioSource;
		private ParticleEmitter particleEmitter;

		public override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody2D>();
			audioSource = GetComponent<AudioSource>();
			particleEmitter = GetComponent<ParticleEmitter>();
		}

		public override void OnUpdate(float delta)
		{
			ProcessThrust();
			ProcessRotation();

			if (Input.IsKeyDown(KeyCode.Escape))
			{
				Application.Shutdown();
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
	}

}

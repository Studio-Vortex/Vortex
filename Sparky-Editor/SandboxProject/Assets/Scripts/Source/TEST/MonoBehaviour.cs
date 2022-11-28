using Sparky;

namespace Sandbox {

	public class MonoBehaviour : Entity
	{
		ParticleEmitter particles;

		protected override void OnCreate()
		{
			particles = GetComponent<ParticleEmitter>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			ProcessMovement();
			EmitParticles();
		}

		void ProcessMovement()
		{
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				velocity += Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity += Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				velocity += Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity += Vector3.Right;

			if (Input.IsKeyDown(KeyCode.LeftShift))
				velocity *= 5;

			transform.Translate(velocity * Time.DeltaTime);
		}

		void EmitParticles()
		{
			if (Input.IsMouseButtonDown(MouseButton.Left))
				particles.Start();
			else
				particles.Stop();
		}
	}

}

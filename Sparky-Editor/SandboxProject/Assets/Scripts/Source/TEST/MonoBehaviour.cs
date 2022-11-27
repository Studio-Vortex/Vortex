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
			if (Input.IsKeyDown(KeyCode.W))
			{
				transform.Translate(Vector3.Forward * deltaTime);
			}

			if (Input.IsMouseButtonDown(MouseButton.Left))
			{
				particles.Start();
				particles.Offset = new Vector3(Input.GetMousePosition() / 200.0f, 0);
			}
			else
			{
				particles.Stop();
			}
		}
	}

}

using Sparky;

namespace Sandbox {

	public class TEST_CLASS : Entity
	{
		protected override void OnUpdate(float delta)
		{
			float speed = 5.0f;

			if (Input.IsKeyDown(KeyCode.W))
				transform.Translate(Vector3.Forward * speed * delta);
			else if (Input.IsKeyDown(KeyCode.S))
				transform.Translate(-Vector3.Forward * speed * delta);
			if (Input.IsKeyDown(KeyCode.A))
				transform.Translate(-Vector3.Right * speed * delta);
			else if (Input.IsKeyDown(KeyCode.D))
				transform.Translate(Vector3.Right * speed * delta);

			Debug.Warn($"Forward: {transform.Forward}");

			if (Input.IsKeyDown(KeyCode.Q))
				transform.Rotate(Vector3.Up * speed * speed * speed * delta);
			else if (Input.IsKeyDown(KeyCode.E))
				transform.Rotate(-Vector3.Up * speed * speed * speed * delta);
		}
	}

}

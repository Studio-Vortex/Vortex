using Vortex;

namespace Sandbox.TEST {

	public class Test : Entity
	{
		public float rotationSpeed;

		void OnCreate()
		{
			
		}

		void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
				transform.Translate(Vector3.Forward * delta);
			if (Input.IsKeyDown(KeyCode.S))
				transform.Translate(Vector3.Back * delta);
			if (Input.IsKeyDown(KeyCode.A))
				transform.Translate(Vector3.Left * delta);
			if (Input.IsKeyDown(KeyCode.D))
				transform.Translate(Vector3.Right * delta);

			if (Input.IsKeyDown(KeyCode.Q))
				transform.Translate(Vector3.Down * delta);
			if (Input.IsKeyDown(KeyCode.E))
				transform.Translate(Vector3.Up * delta);

			if (Input.IsKeyDown(KeyCode.Space))
				transform.Rotate(Vector3.One * rotationSpeed * delta);
		}
	}

}

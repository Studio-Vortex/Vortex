using Vortex;

namespace Sandbox {

	public class CameraController : Entity
	{
		void OnCreate()
		{

		}

		void OnUpdate(float delta)
		{
			MoveCamera();
			RotateCamera();
		}

		void MoveCamera()
		{
			Vector3 movement = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				movement += transform.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				movement -= transform.Forward;

			if (Input.IsKeyDown(KeyCode.A))
				movement -= transform.Right;
			else if (Input.IsKeyDown(KeyCode.D))
				movement += transform.Right;

			transform.Translate(movement * Time.DeltaTime);
		}

		void RotateCamera()
		{
			Vector3 rotation = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.Up))
				rotation += Vector3.Right;
			else if (Input.IsKeyDown(KeyCode.Down))
				rotation += Vector3.Left;

			if (Input.IsKeyDown(KeyCode.Left))
				rotation += Vector3.Up;
			else if (Input.IsKeyDown(KeyCode.Right))
				rotation += Vector3.Down;

			transform.Rotate(rotation);
		}
	}

}

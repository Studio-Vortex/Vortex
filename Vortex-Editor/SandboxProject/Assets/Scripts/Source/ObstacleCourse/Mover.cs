using Vortex;

namespace Sandbox.ObstacleCourse {

	public class Mover : Entity
	{
		public float moveSpeed;
		public Vector3 cameraOffset;

		Vector3 speed;
		RigidBody rigidbody;
		Entity camera;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
			camera = FindEntityByName("Camera");
		}

		protected override void OnUpdate(float deltaTime)
		{
			camera.transform.Translation = transform.Translation + cameraOffset;

			MovePlayer();
		}

		private void MovePlayer()
		{
			speed = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				speed = Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				speed = Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				speed = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				speed = Vector3.Right;

			Vector3 force = speed * moveSpeed * Time.DeltaTime;
			rigidbody.AddForce(force);
		}
	}

}

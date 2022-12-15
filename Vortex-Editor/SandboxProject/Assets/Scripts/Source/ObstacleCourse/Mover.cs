using Vortex;

namespace Sandbox.ObstacleCourse {

	public class Mover : Entity
	{
		public float moveSpeed;
		public Vector3 cameraOffset;

		Vector3 speed;
		RigidBody rigidbody;
		BoxCollider collider;
		Entity camera;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
			collider = GetComponent<BoxCollider>();
			camera = FindEntityByName("Camera");
		}

		protected override void OnUpdate(float deltaTime)
		{
			camera.transform.Translation = transform.Translation + cameraOffset;

			if (Time.Elapsed > 30.0f)
			{
				SceneManager.LoadScene(SceneManager.GetActiveScene());
			}

			MovePlayer();
			CheckForCoins();
		}

		void MovePlayer()
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

		void CheckForCoins()
		{
			Physics.Raycast(transform.Translation + new Vector3(transform.Scale.X * collider.HalfSize.X + 0.25f, 0, 0), Vector3.Right, 1, out RaycastHit _);
			Physics.Raycast(transform.Translation - new Vector3(transform.Scale.X * collider.HalfSize.X - 0.25f, 0, 0), Vector3.Left, 1, out RaycastHit _);
			Physics.Raycast(transform.Translation - new Vector3(0, 0, transform.Scale.Z * collider.HalfSize.Z - 0.25f), Vector3.Forward, 1, out RaycastHit _);
			Physics.Raycast(transform.Translation + new Vector3(0, 0, transform.Scale.Z * collider.HalfSize.Z + 0.25f), Vector3.Back, 1, out RaycastHit _);
		}
	}

}

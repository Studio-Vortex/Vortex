using Sparky;

namespace Sandbox {

	public class Mover : Entity
	{
		public float moveSpeed;

		Vector3 speed;
		RigidBody rigidbody;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
		}

		protected override void OnUpdate(float deltaTime)
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

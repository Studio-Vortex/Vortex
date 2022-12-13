using Vortex;

namespace Sandbox {

	public class CubeController : Entity
	{
		public float moveSpeed;

		RigidBody rigidbody;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
		}

		protected override void OnUpdate(float delta)
		{
			rigidbody.AddForce(Vector3.Forward * moveSpeed * Time.DeltaTime);

			Vector3 force = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.A))
				force = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				force = Vector3.Right;

			rigidbody.AddForce(force * moveSpeed * Time.DeltaTime);
		}
	}

}

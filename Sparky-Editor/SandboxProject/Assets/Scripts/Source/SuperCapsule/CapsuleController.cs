using Sparky;

namespace Sandbox.SuperCapsule {

	public class CapsuleController : Entity
	{
		public float moveSpeed;
		public float jumpForce;
		public float resetPoint;

		Vector3 startPosition;
		RigidBody rigidbody;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
			startPosition = transform.Translation;
		}

		protected override void OnUpdate(float delta)
		{
			MovePlayer();
			Jump();

			ResetPlayerIfNeeded();
		}

		void MovePlayer()
		{
			Vector3 displacement = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				displacement += Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				displacement += Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				displacement += Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				displacement += Vector3.Right;

			rigidbody.AddForce(displacement * moveSpeed * Time.DeltaTime);
		}

		void Jump()
		{
			bool isGrounded = Physics.Raycast(transform.Translation - (transform.Up * 1.02f), Vector3.Down, 0.01f, out RaycastHit hitInfo);

			if (Input.IsKeyDown(KeyCode.Space) && isGrounded)
				rigidbody.AddForce(Vector3.Up * jumpForce, ForceMode.Impulse);
		}

		void ResetPlayerIfNeeded()
		{
			if (rigidbody.Translation.Y < resetPoint)
			{
				rigidbody.Translation = startPosition;
			}
		}
	}

}

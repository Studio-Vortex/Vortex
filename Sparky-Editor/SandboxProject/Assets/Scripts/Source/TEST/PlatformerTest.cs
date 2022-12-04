using Sparky;

namespace Sandbox {

	public class PlatformerTest : Entity
	{
		public float speed;
		public float jumpForce;
		public bool isGrounded;

		RigidBody2D rigidbody;
		CircleCollider2D collider;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody2D>();
			collider = GetComponent<CircleCollider2D>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			ProcessMovement();
			Jump();
		}

		void ProcessMovement()
		{
			if (Input.IsKeyDown(KeyCode.A))
				rigidbody.ApplyForce(Vector2.Left * speed * Time.DeltaTime, true);
			else if (Input.IsKeyDown(KeyCode.D))
				rigidbody.ApplyForce(Vector2.Right * speed * Time.DeltaTime, true);
		}

		void Jump()
		{
			Vector2 groundPoint = transform.Translation.XY - new Vector2(0, collider.Radius + 0.05f);
			Physics2D.Raycast(transform.Translation.XY, groundPoint, out RayCastHit2D hitInfo);

			isGrounded = hitInfo.Hit;

			if (Input.IsKeyDown(KeyCode.Space) && isGrounded)
			{
				rigidbody.ApplyForce(Vector2.Up * jumpForce, true);
			}
		}
	}

}

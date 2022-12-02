using Sparky;
using System.Diagnostics.Contracts;

namespace Sandbox {

	public class CharacterControllerTest : Entity
	{
		public float jumpForce;
		public Vector3 speed;

		CharacterController controller;

		protected override void OnCreate()
		{
			controller = GetComponent<CharacterController>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			Vector3 movement = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				movement = Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				movement = Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				movement = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				movement = Vector3.Right;

			controller.Move(movement * speed * Time.DeltaTime);

			if (Input.IsKeyDown(KeyCode.Space) && controller.IsGrounded)
			{
				controller.Jump(jumpForce);
			}
		}
	}

}

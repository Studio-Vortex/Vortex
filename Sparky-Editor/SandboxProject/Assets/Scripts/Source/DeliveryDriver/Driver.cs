using Sparky;
using System;

namespace Sandbox {

	public class Driver : Entity
	{
		public float moveSpeed;
		public float steerSpeed;

		protected override void OnCreate()
		{
		}

		protected override void OnUpdate(float delta)
		{
			ProcessRotation();
			ProcessMovement();
		}

		void ProcessRotation()
		{
			float steerAmount = 0.0f;

			if (Input.IsKeyDown(KeyCode.A))
				steerAmount = -Math.Abs(steerSpeed);
			else if (Input.IsKeyDown(KeyCode.D))
				steerAmount = Math.Abs(steerSpeed);

			transform.Rotate(Vector3.Forward * steerAmount * Time.DeltaTime);
		}

		void ProcessMovement()
		{
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				velocity = transform.Up;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity = -transform.Up;

			transform.Translate(velocity * moveSpeed * Time.DeltaTime);
		}
	}

}

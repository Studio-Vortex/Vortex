using Sparky;
using System;

namespace Sandbox {

	public class PhysicsTest : Entity
	{
		RigidBody rigidbody;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (Input.IsKeyDown(KeyCode.W))
				rigidbody.Translation += transform.Forward * Time.DeltaTime;
			else if (Input.IsKeyDown(KeyCode.S))
				rigidbody.Translation -= transform.Forward * Time.DeltaTime;

			if (Input.IsKeyDown(KeyCode.A))
				rigidbody.Translation -= transform.Right * Time.DeltaTime;
			else if (Input.IsKeyDown(KeyCode.D))
				rigidbody.Translation += transform.Right * Time.DeltaTime;

			if (Input.IsKeyDown(KeyCode.R))
				rigidbody.Rotation += Vector3.Right * Time.DeltaTime;

			if (Input.IsKeyDown(KeyCode.Space))
				rigidbody.AddForce(Vector3.Up * 2500 * Time.DeltaTime);

			if (Input.IsKeyDown(KeyCode.LeftShift))
				rigidbody.AddTorque(Vector3.Right * 50.0f * Time.DeltaTime);
		}
	}

}

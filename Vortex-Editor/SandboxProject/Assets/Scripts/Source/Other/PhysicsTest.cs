using Vortex;
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
				transform.Translation += transform.Forward * Time.DeltaTime;
			else if (Input.IsKeyDown(KeyCode.S))
				transform.Translation -= transform.Forward * Time.DeltaTime;

			if (Input.IsKeyDown(KeyCode.A))
				transform.Translation -= transform.Right * Time.DeltaTime;
			else if (Input.IsKeyDown(KeyCode.D))
				transform.Translation += transform.Right * Time.DeltaTime;

			if (Input.IsKeyDown(KeyCode.R))
				transform.Rotate(Vector3.Right * Time.DeltaTime);

			if (Input.IsKeyDown(KeyCode.Space))
				rigidbody.AddForce(Vector3.Up * 2500 * Time.DeltaTime);

			if (Input.IsKeyDown(KeyCode.LeftShift))
				rigidbody.AddTorque(Vector3.Right * 50.0f * Time.DeltaTime);
		}
	}

}

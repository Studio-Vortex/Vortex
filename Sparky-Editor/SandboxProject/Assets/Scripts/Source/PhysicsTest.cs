using Sparky;
using System;

namespace Sandbox {

	public class PhysicsTest : Entity
	{
		RigidBody rigidBody;

		protected override void OnCreate()
		{
			rigidBody = GetComponent<RigidBody>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (Input.IsKeyDown(KeyCode.W))
				rigidBody.AddForce(Vector3.Forward);
			else if (Input.IsKeyDown(KeyCode.S))
				rigidBody.AddForce(Vector3.Back);

			if (Input.IsKeyDown(KeyCode.A))
				rigidBody.AddForce(Vector3.Left);
			else if (Input.IsKeyDown(KeyCode.D))
				rigidBody.AddForce(Vector3.Right);

			if (Input.IsKeyDown(KeyCode.Space))
				rigidBody.AddForce(Vector3.Up * 2000.0f * Time.DeltaTime);
		}
	}

}

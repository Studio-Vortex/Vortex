using Sparky;
using System;

namespace Sandbox {

	public class PhysicsTest : Entity
	{
		public float speed = 2000f;
		RigidBody rigidBody;

		protected override void OnCreate()
		{
			rigidBody = GetComponent<RigidBody>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (Input.IsKeyDown(KeyCode.W))
				rigidBody.AddForce(Vector3.Forward * speed * Time.DeltaTime);
			else if (Input.IsKeyDown(KeyCode.S))
				rigidBody.AddForce(Vector3.Back * speed * Time.DeltaTime);

			if (Input.IsKeyDown(KeyCode.A))
				rigidBody.AddForce(Vector3.Left * speed * Time.DeltaTime);
			else if (Input.IsKeyDown(KeyCode.D))
				rigidBody.AddForce(Vector3.Right * speed * Time.DeltaTime);

			if (Input.IsKeyDown(KeyCode.Space))
				rigidBody.AddForce(Vector3.Up * speed * Time.DeltaTime);
		}
	}

}

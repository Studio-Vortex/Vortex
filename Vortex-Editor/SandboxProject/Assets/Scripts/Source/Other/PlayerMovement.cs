using Vortex;
using System;

namespace Sandbox {

	public class PlayerMovement : Entity
	{
		public float Speed;
		public Vector3 CameraOffset;

		RigidBody rigidbody;
		Entity m_Camera;

		protected override void OnCreate()
		{
			rigidbody = GetComponent<RigidBody>();
			m_Camera = FindEntityByName("Camera");
		}

		protected override void OnUpdate(float delta)
		{
			UpdateSpeed();
			ProcessMovement();
			m_Camera.transform.Translation = transform.Translation + CameraOffset;
		}

		void UpdateSpeed()
		{
			Speed += Time.DeltaTime;
		}

		void ProcessMovement()
		{
			Vector3 velocity = Vector3.Zero;
			if (Input.IsKeyDown(KeyCode.W))
				velocity = Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity = Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				velocity = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity = Vector3.Right;

			rigidbody.AddForce(velocity * Speed * Time.DeltaTime);
		}
	}

}

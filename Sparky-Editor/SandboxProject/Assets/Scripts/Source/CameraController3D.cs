using System;

using Sparky;

namespace Sandbox {

	public class CameraController3D : Entity
	{
		public Vector3 Speed;
		public Vector3 RotationSpeed;
		private Vector3 Velocity;
		private Vector3 m_Rotation;
		
		public bool FixedRotation;

		public void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
				Velocity = Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				Velocity = Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				Velocity = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				Velocity = Vector3.Right;

			if (!FixedRotation)
			{
				if (Input.IsKeyDown(KeyCode.Q))
					m_Rotation.Y = 1.0f;
				if (Input.IsKeyDown(KeyCode.E))
					m_Rotation.Y = -1.0f;
			}

			if (Input.IsKeyDown(KeyCode.LeftShift))
				Velocity *= 2.0f;

			Velocity *= Speed * delta;
			m_Rotation *= RotationSpeed * delta;

			transform.Translation += Velocity;
			transform.Rotation += m_Rotation;
		}
	}

}

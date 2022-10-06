using System;
using Sparky;

namespace Sandbox {

	public class CameraController3D : Entity
	{
		public Vector3 Speed;
		public Vector3 RotationSpeed;
		private Vector3 m_Velocity;
		private Vector3 m_Rotation;
		
		public bool FixedRotation;

		public override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
				m_Velocity = Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				m_Velocity = Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				m_Velocity = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				m_Velocity = Vector3.Right;

			if (!FixedRotation)
			{
				if (Input.IsKeyDown(KeyCode.Left))
					m_Rotation.Y = 1.0f;
				if (Input.IsKeyDown(KeyCode.Right))
					m_Rotation.Y = -1.0f;

				if (Input.IsKeyDown(KeyCode.Up))
					m_Rotation.X = 1.0f;
				if (Input.IsKeyDown(KeyCode.Down))
					m_Rotation.X = -1.0f;
			}

			if (Input.IsKeyDown(KeyCode.LeftShift))
				m_Velocity *= 2.0f;

			m_Velocity *= Speed * delta;
			m_Rotation *= RotationSpeed * delta;

			transform.Translation += m_Velocity;
			transform.Rotation += m_Rotation;
		}
	}

}

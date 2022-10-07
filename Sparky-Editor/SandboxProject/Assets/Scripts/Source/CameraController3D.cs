using System;
using Sparky;

namespace Sandbox {

	public class CameraController3D : Entity
	{
		public float MaxRoll_Up = 60.0f;
		public float MaxRoll_Down = -60.0f;
		public float ShiftModifer = 2.0f;
		public Vector3 Speed = new Vector3(4.0f, 0.0f, 4.0f);
		public Vector3 RotationSpeed = new Vector3(100.0f, 100.0f, 0.0f);
		
		public bool FixedRotation;

		private Vector3 m_Velocity;
		private Vector3 m_Rotation;

		public override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
			{
				if (transform.Rotation.Y > 270.0f || transform.Rotation.Y < 90.0f)
					m_Velocity = Vector3.Forward;
				else if (transform.Rotation.Y > 90.0f || transform.Rotation.Y < 270.0f)
					m_Velocity = Vector3.Back;
			}
			else if (Input.IsKeyDown(KeyCode.S))
			{
				if (transform.Rotation.Y < 270.0f || transform.Rotation.Y > 90.0f)
					m_Velocity = Vector3.Back;
				else if (transform.Rotation.Y < 90.0f || transform.Rotation.Y > 270.0f)
					m_Velocity = Vector3.Forward;
			}

			if (Input.IsKeyDown(KeyCode.A))
				m_Velocity = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				m_Velocity = Vector3.Right;

			if (!FixedRotation)
			{
				ProcessRotation();
				Debug.Log("ProcessRotation()");
			}

			if (Input.IsKeyDown(KeyCode.LeftShift))
				m_Velocity *= ShiftModifer;

			m_Velocity *= Speed * delta;
			m_Rotation *= RotationSpeed * delta;

			transform.Translation += m_Velocity;
			transform.Rotation += m_Rotation;

			if (transform.Rotation.Y >= 360.0f)
				transform.Rotation = new Vector3(transform.Rotation.X, 0.0f, transform.Rotation.Z);
			if (transform.Rotation.Y <= 0.0f)
				transform.Rotation = new Vector3(transform.Rotation.X, 0.0f, transform.Rotation.Z);

			transform.Rotation = new Vector3(transform.Rotation.X, transform.Rotation.Y, transform.Rotation.Z);
		}

		private void ProcessRotation()
		{
			if (Input.IsKeyDown(KeyCode.Left))
				m_Rotation.Y = 1.0f;
			if (Input.IsKeyDown(KeyCode.Right))
				m_Rotation.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.Up))
				m_Rotation.X = 1.0f;
			if (Input.IsKeyDown(KeyCode.Down))
				m_Rotation.X = -1.0f;

			if (transform.Rotation.X >= MaxRoll_Down)
			{
				float newRoll = Math.Min(MaxRoll_Up, transform.Rotation.X);
				transform.Rotation = new Vector3(newRoll, transform.Rotation.Y, transform.Rotation.Z);
			}
			if (transform.Rotation.X <= MaxRoll_Down)
			{
				float newRoll = Math.Max(MaxRoll_Down, transform.Rotation.X);
				transform.Rotation = new Vector3(newRoll, transform.Rotation.Y, transform.Rotation.Z);
			}
		}
	}

}

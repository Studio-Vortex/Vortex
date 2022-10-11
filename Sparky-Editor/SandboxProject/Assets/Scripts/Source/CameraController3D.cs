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
				m_Velocity = Vector3.Forward;
			else if (Input.IsKeyDown(KeyCode.S))
				m_Velocity = Vector3.Back;

			if (Input.IsKeyDown(KeyCode.A))
				m_Velocity = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				m_Velocity = Vector3.Right;

			if (Input.IsKeyDown(KeyCode.Q))
				m_Velocity = Vector3.Up;
			else if (Input.IsKeyDown(KeyCode.E))
				m_Velocity = Vector3.Down;

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

			if (transform.Rotation.X >= MaxRoll_Up)
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

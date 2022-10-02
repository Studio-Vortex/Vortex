using System;
using Sparky;

namespace Sandbox {

	public class Paddle : Entity
	{
		public float Speed;
		private Vector3 m_Velocity;

		public void OnCreate()
		{
			Debug.Log("Welcome to Sparky!");
			Debug.Info("Welcome to Sparky!");
			Debug.Warn("Welcome to Sparky!");
			Debug.Error("Welcome to Sparky!");
			Debug.Critical("Welcome to Sparky!");
		}

		public void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
				m_Velocity.Y = 1.0f;
			if (Input.IsKeyDown(KeyCode.S))
				m_Velocity.Y = -1.0f;

			m_Velocity *= Speed * delta;
			transform.Translation += m_Velocity;

			if (transform.Translation.Y > 3.25f)
				transform.Translation = new Vector3(transform.Translation.X, 3.25f, transform.Translation.Z);
			if (transform.Translation.Y < -3.25f)
				transform.Translation = new Vector3(transform.Translation.X, -3.25f, transform.Translation.Z);

			if (Input.IsKeyDown(KeyCode.Space))
				Renderer.SetClearColor(new Vector3(0.2f, 0.2f, 0.8f));
		}
	}

}

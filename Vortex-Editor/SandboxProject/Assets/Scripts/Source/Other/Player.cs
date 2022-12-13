using System;
using Vortex;

namespace Sandbox {

	public class Player : Entity
	{
		public Vector3 Velocity;
		public float Speed;
		public float Time;
		public RigidBody2D rb;
		public bool ShowRaycast;
		private Camera2D m_Camera;
		private SpriteRenderer m_SpriteRenderer;

		protected override void OnCreate()
		{
			Time = 0.0f;
			rb = GetComponent<RigidBody2D>();
			m_Camera = FindEntityByName("Camera").As<Camera2D>();
			m_SpriteRenderer = GetComponent<SpriteRenderer>();

			Speed *= 1.5f;
		}

		protected override void OnUpdate(float delta)
		{
			Time += delta;

			Velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				Velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				Velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				Velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				Velocity.X = 1.0f;

			m_Camera.DistanceToPlayer += delta;

			if (m_Camera.DistanceToPlayer > 20.0f)
				m_Camera.DistanceToPlayer = 20.0f;

			if (Input.IsKeyDown(KeyCode.Q))
				m_Camera.DistanceToPlayer += 2.0f * delta;
			if (Input.IsKeyDown(KeyCode.E))
				m_Camera.DistanceToPlayer -= 2.0f * delta;

			Entity other = Physics2D.Raycast(transform.Translation.XY, transform.Translation.XY + Vector2.Down, out RayCastHit2D hit, ShowRaycast);

			if (hit.Hit)
			{
				m_SpriteRenderer.Color = new Vector4(1.0f, 0.0f, 0.0f, 1.0f);
				if (hit.Tag == "Floor")
					Debug.Log("Floor Hit");
			}
			else
				m_SpriteRenderer.Color = new Vector4(1.0f);

			Velocity *= Speed * delta;
			rb.ApplyForce(Velocity.XY, true);
		}
	}

}

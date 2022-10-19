using System;
using Sparky;

namespace Sandbox {

	public class TestClass : Entity
	{
		public float Speed;
		public Vector3 Velocity;

		public override void OnCreate()
		{
			// Called once before the first frame

			GetComponent<AudioSource>().Play();
		}

		public override void OnUpdate(float delta)
		{
			// Called once every frame

			Velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				Velocity.Y = 1.0f;
			else if (Input.IsKeyDown(KeyCode.S))
				Velocity.Y = -1.0f;

			if (Input.IsKeyDown(KeyCode.A))
				Velocity.X = -1.0f;
			else if (Input.IsKeyDown(KeyCode.D))
				Velocity.X = 1.0f;

			if (Input.IsKeyDown(KeyCode.Space))
				SpawnBullet();

			transform.Translate(Velocity * Speed * delta);
		}

		private void SpawnBullet()
		{
			Entity bullet = new Entity("Bullet");
			RigidBody2D rb = bullet.AddComponent<RigidBody2D>();
			rb.Type = RigidBody2DType.Dynamic;
			bullet.AddComponent<CircleCollider2D>();
			CircleRenderer circle = bullet.AddComponent<CircleRenderer>();
			rb.ApplyForce(Vector2.Up, true);
			circle.Color = new Vector4(1.0f, 1.0f, 0.2f, 1.0f);
		}
	}

}

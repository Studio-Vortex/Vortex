using System;
using Sparky;

namespace Sandbox {

	public class TestClass : Entity
	{
		public float Speed;
		public float ControllerDeadzone = 0.15f;
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

			if (Input.IsKeyDown(KeyCode.W) || Input.GetGamepadAxis(Gamepad.AxisLeftY) < -ControllerDeadzone)
				Velocity.Y = 1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftY);
			else if (Input.IsKeyDown(KeyCode.S) || Input.GetGamepadAxis(Gamepad.AxisLeftY) > ControllerDeadzone)
				Velocity.Y = -1.0f * -Input.GetGamepadAxis(Gamepad.AxisLeftY);

			if (Input.IsKeyDown(KeyCode.A) || Input.GetGamepadAxis(Gamepad.AxisLeftX) < -ControllerDeadzone)
				Velocity.X = -1.0f * -Input.GetGamepadAxis(Gamepad.AxisLeftX);
			else if (Input.IsKeyDown(KeyCode.D) || Input.GetGamepadAxis(Gamepad.AxisLeftX) > ControllerDeadzone)
				Velocity.X = 1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftX);

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

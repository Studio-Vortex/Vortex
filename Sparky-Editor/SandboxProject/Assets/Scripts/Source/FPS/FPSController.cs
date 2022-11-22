using Sparky;
using System;

namespace Sandbox {

	public class FPSController : Entity
	{
		public float MaxRoll_Up = 60f;
		public float MaxRoll_Down = -60f;
		public float TimeBetweenShots = 1f;
		public float BulletSpeed = 50f;
		public Vector3 WalkSpeed = new Vector3(4f, 0f, 4f);
		public Vector3 RunSpeed = new Vector3(10f, 0f, 10f);
		public Vector3 RotationSpeed = new Vector3(100f, 100f, 0f);

		float m_TimeBetweenShot;
		Vector3 m_Velocity;
		Vector3 m_Rotation;
		AudioSource gunshotSound;
		ParticleEmitter muzzleBlast;
		Entity bulletContainter;

		const float ShiftModifer = 2.0f;
		const float ControllerDeadzone = 0.15f;

		protected override void OnCreate()
		{
			Input.ShowMouseCursor = false;
			m_TimeBetweenShot = TimeBetweenShots;
			gunshotSound = FindEntityByName("Camera").GetComponent<AudioSource>();
			muzzleBlast = FindEntityByName("Gun").GetComponent<ParticleEmitter>();
			bulletContainter = FindEntityByName("Bullet Container");
		}

		protected override void OnUpdate(float delta)
		{
			ProcessMovement();
			ProcessRotation();

			m_TimeBetweenShot -= Time.DeltaTime;

			muzzleBlast.Stop();

			float rightTrigger = Input.GetGamepadAxis(Gamepad.AxisRightTrigger);
			if (rightTrigger > 0.0f && m_TimeBetweenShot <= 0.0f)
			{
				FireBullet();
			}

			Vector3 speed = Input.IsGamepadButtonDown(Gamepad.LeftStick) ? RunSpeed : WalkSpeed;

			m_Velocity *= speed * delta;
			m_Rotation *= RotationSpeed * delta;

			transform.Translation += m_Velocity;
			transform.Rotation += m_Rotation;

		}

		void ProcessMovement()
		{
			Vector3 speed = Input.IsGamepadButtonDown(Gamepad.LeftStick) ? RunSpeed : WalkSpeed;

			if (Input.GetGamepadAxis(Gamepad.AxisLeftY) < -ControllerDeadzone)
				transform.Translate(-transform.Forward * speed * Time.DeltaTime * Input.GetGamepadAxis(Gamepad.AxisLeftY));
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftY) > ControllerDeadzone)
				transform.Translate(-transform.Forward * speed * Time.DeltaTime * Input.GetGamepadAxis(Gamepad.AxisLeftY));

			if (Input.GetGamepadAxis(Gamepad.AxisLeftX) < -ControllerDeadzone)
				transform.Translate(transform.Right * speed * Time.DeltaTime * Input.GetGamepadAxis(Gamepad.AxisLeftX));
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftX) > ControllerDeadzone)
				transform.Translate(transform.Right * speed * Time.DeltaTime * Input.GetGamepadAxis(Gamepad.AxisLeftX));

			if (Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > ControllerDeadzone)
				m_Velocity.Y = 1.0f * Input.GetGamepadAxis(Gamepad.AxisRightTrigger);
			else if (Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > ControllerDeadzone)
				m_Velocity.Y = -1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftTrigger);

			if (Input.IsGamepadButtonDown(Gamepad.LeftStick))
				m_Velocity *= ShiftModifer;
		}

		void ProcessRotation()
		{
			float rightAxisX = -Input.GetGamepadAxis(Gamepad.AxisRightX);
			if (rightAxisX < -ControllerDeadzone || rightAxisX > ControllerDeadzone)
				m_Rotation.Y = rightAxisX;

			float rightAxisY = -Input.GetGamepadAxis(Gamepad.AxisRightY);
			if (rightAxisY < -ControllerDeadzone || rightAxisY > ControllerDeadzone)
				m_Rotation.X = rightAxisY;

			if (transform.Rotation.X >= MaxRoll_Up)
			{
				float roll = Math.Min(MaxRoll_Up, transform.Rotation.X);
				transform.Rotation = new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z);
			}
			if (transform.Rotation.X <= MaxRoll_Down)
			{
				float roll = Math.Max(MaxRoll_Down, transform.Rotation.X);
				transform.Rotation = new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z);
			}
		}

		void FireBullet()
		{
			Entity entity = new Entity("Bullet");
			entity.transform.Translation = transform.Translation + (transform.Forward * 2.0f) + (transform.Right * 0.5f) + (transform.Up * 0.25f);
			entity.transform.Scale = new Vector3(0.5f);

			entity.AddComponent<MeshRenderer>();
			entity.AddComponent<BoxCollider>();

			RigidBody rigidbody = entity.AddComponent<RigidBody>();
			rigidbody.BodyType = RigidBodyType.Dynamic;
			rigidbody.Velocity = transform.Forward * BulletSpeed;
			rigidbody.AngularVelocity = new Vector3(RandomDevice.RangedFloat(0, 1) * BulletSpeed);

			gunshotSound.Play();
			muzzleBlast.Velocity = transform.Forward;
			muzzleBlast.Offset = transform.Forward;
			muzzleBlast.Start();

			bulletContainter.AddChild(entity);

			m_TimeBetweenShot = TimeBetweenShots;
		}
	}

}

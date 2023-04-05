using Vortex;
using System;

namespace Sandbox
{

	public class FPSController : Entity
	{
		public float maxLookUp = 60f;
		public float maxLookDown = -60f;
		public float timeBetweenShots = 1f;
		public float gunPower = 50f;
		public Vector3 walkSpeed = new Vector3(4f, 0f, 4f);
		public Vector3 runSpeed = new Vector3(10f, 0f, 10f);
		public Vector3 rotationSpeed = new Vector3(100f, 100f, 0f);

		float m_TimeBetweenShot;
		Vector3 m_Velocity;
		Vector3 m_Rotation;
		AudioSource gunshotSound;
		ParticleEmitter muzzleBlast;

		const float ShiftModifer = 2.0f;
		const float ControllerDeadzone = 0.15f;

		protected override void OnCreate()
		{
			Input.SetCursorMode(CursorMode.Locked);
			m_TimeBetweenShot = timeBetweenShots;
			gunshotSound = Scene.FindEntityByName("Camera").GetComponent<AudioSource>();
			muzzleBlast = Scene.FindEntityByName("Gun").GetComponent<ParticleEmitter>();

			/*Entity container = FindEntityByName("Container");

			Entity[] children = container.Children;*/
			/*foreach (Entity child in children)
			{
				MeshRenderer meshRenderer = child.GetComponent<MeshRenderer>();
				float x = RandomDevice.RangedFloat(0, 1);
				float y = RandomDevice.RangedFloat(0, 1);
				float z = RandomDevice.RangedFloat(0, 1);
				meshRenderer.Material.Albedo = new Vector3(x, y, z);
			}*/
		}

		protected override void OnUpdate(float delta)
		{
			ProcessMovement();
			ProcessRotation();

			m_TimeBetweenShot -= Time.DeltaTime;

			muzzleBlast.Stop();

			float rightTrigger = Input.GetGamepadAxis(GamepadAxis.LeftTrigger);
			if (rightTrigger > 0.0f && m_TimeBetweenShot <= 0.0f)
			{
				PlayEffects();
				FireBullet();
			}

			Vector3 speed = Input.IsGamepadButtonDown(GamepadButton.LeftStick) ? runSpeed : walkSpeed;

			m_Velocity *= speed * delta;
			m_Rotation *= rotationSpeed * delta;

			transform.Translation += m_Velocity;
			transform.EulerAngles += m_Rotation;
		}

		void ProcessMovement()
		{
			Vector3 speed = Input.IsGamepadButtonDown(GamepadButton.LeftStick) ? runSpeed : walkSpeed;

			if (Input.GetGamepadAxis(GamepadAxis.LeftTrigger) < -ControllerDeadzone)
				transform.Translate(-transform.Forward * speed * Time.DeltaTime * Input.GetGamepadAxis(GamepadAxis.LeftY));
			else if (Input.GetGamepadAxis(GamepadAxis.LeftTrigger) > ControllerDeadzone)
				transform.Translate(-transform.Forward * speed * Time.DeltaTime * Input.GetGamepadAxis(GamepadAxis.LeftY));

			if (Input.GetGamepadAxis(GamepadAxis.LeftX) < -ControllerDeadzone)
				transform.Translate(transform.Right * speed * Time.DeltaTime * Input.GetGamepadAxis(GamepadAxis.LeftX));
			else if (Input.GetGamepadAxis(GamepadAxis.LeftX) > ControllerDeadzone)
				transform.Translate(transform.Right * speed * Time.DeltaTime * Input.GetGamepadAxis(GamepadAxis.LeftX));

			if (Input.GetGamepadAxis(GamepadAxis.RightTrigger) > ControllerDeadzone)
				m_Velocity.Y = 1.0f * Input.GetGamepadAxis(GamepadAxis.RightTrigger);
			else if (Input.GetGamepadAxis(GamepadAxis.LeftTrigger) > ControllerDeadzone)
				m_Velocity.Y = -1.0f * Input.GetGamepadAxis(GamepadAxis.LeftTrigger);

			if (Input.IsGamepadButtonDown(GamepadButton.LeftStick))
				m_Velocity *= ShiftModifer;
		}

		void ProcessRotation()
		{
			float rightAxisX = -Input.GetGamepadAxis(GamepadAxis.RightX);
			if (rightAxisX < -ControllerDeadzone || rightAxisX > ControllerDeadzone)
				m_Rotation.Y = rightAxisX;

			float rightAxisY = -Input.GetGamepadAxis(GamepadAxis.RightY);
			if (rightAxisY < -ControllerDeadzone || rightAxisY > ControllerDeadzone)
				m_Rotation.X = rightAxisY;

			if (transform.Rotation.X >= maxLookUp)
			{
				float roll = Math.Min(maxLookUp, transform.Rotation.X);
				transform.Rotation *= new Quaternion(new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z));
			}
			if (transform.Rotation.X <= maxLookDown)
			{
				float roll = Math.Max(maxLookDown, transform.Rotation.X);
				transform.Rotation *= new Quaternion(new Vector3(roll, transform.Rotation.Y, transform.Rotation.Z));
			}
		}

		void PlayEffects()
		{
			gunshotSound.Play();

			muzzleBlast.Velocity = transform.Forward + transform.Up;
			muzzleBlast.Offset = transform.Forward * 2.0f;
			muzzleBlast.Start();
		}

		void FireBullet()
		{
			Entity bullet = Scene.CreateEntity("Bullet");
			bullet.transform.Translation = transform.Translation + (transform.Forward * 2.0f) + (transform.Right * 0.5f) + (transform.Up * 0.25f);
			bullet.transform.Scale = new Vector3(0.5f);

			StaticMeshRenderer meshRenderer = bullet.AddComponent<StaticMeshRenderer>();
			meshRenderer.MeshType = MeshType.Sphere;
			Material material = meshRenderer.GetSubmesh(0).Material;
			material.Albedo = Color.Red;
			bullet.AddComponent<SphereCollider>();

			RigidBody rigidbody = bullet.AddComponent<RigidBody>();
			rigidbody.BodyType = RigidBodyType.Dynamic;
			rigidbody.AddForce(transform.Forward * gunPower, ForceMode.Impulse);

			m_TimeBetweenShot = timeBetweenShots;
		}
	}

}
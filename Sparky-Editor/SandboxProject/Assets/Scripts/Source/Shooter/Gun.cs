﻿using Sparky;

namespace Sandbox {

	public class Gun : Entity
	{
		public float timeBetweenShots = 1.4f;
		public float bulletSpeed = 180f;
		public float normalFOV = 60f;
		public float zoomedFOV = 40f;
		public uint bulletsPerShot = 1;
		public uint ammo = 10;

		float timeToWait = 0f;

		Vector3 startPosition;
		Vector3 startRotation;
		Vector3 zoomedPosition;
		Vector3 zoomedRotation;

		Vector3 eyeLookDirection;
		Entity eyes;
		Vector3 playerLookDirection;
		Entity player;

		Camera camera;

		AudioSource gunshotSound;
		AudioSource emptyGunSound;
		ParticleEmitter muzzleBlast;

		protected override void OnCreate()
		{
			eyes = FindEntityByName("Eyes");
			player = FindEntityByName("Player");
			camera = FindEntityByName("Camera").GetComponent<Camera>();
			emptyGunSound = FindEntityByName("Empty Gun Sound").GetComponent<AudioSource>();
			gunshotSound = GetComponent<AudioSource>();
			muzzleBlast = GetComponent<ParticleEmitter>();
			startPosition = transform.Translation;
			startRotation = transform.Rotation;
			Transform zoomedTransform = FindEntityByName("Zoomed Transform").transform;
			zoomedPosition = zoomedTransform.Translation;
			zoomedRotation = zoomedTransform.Rotation;
		}

		protected override void OnUpdate(float deltaTime)
		{
			UpdateState();
			ProcessFire();
			ProcessZoom();

			timeToWait -= Time.DeltaTime;
		}

		void ProcessZoom()
		{
			bool rightMouseButtonPressed = Input.IsMouseButtonDown(MouseButton.Right);
			bool rightMouseButtonReleased = Input.IsMouseButtonUp(MouseButton.Right);
			bool leftTriggerPressed = Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > 0f;

			if (rightMouseButtonPressed || leftTriggerPressed)
			{
				camera.FieldOfView = zoomedFOV;
				transform.Translation = zoomedPosition;
				transform.Rotation = zoomedRotation;
			}
			else if (rightMouseButtonReleased)
			{
				camera.FieldOfView = normalFOV;
				transform.Translation = startPosition;
				transform.Rotation = startRotation;
			}
		}

		void ProcessFire()
		{
			bool waitTimeOver = timeToWait <= 0f;
			bool leftMouseButtonPressed = Input.IsMouseButtonDown(MouseButton.Left);
			bool leftMouseButtonReleased = Input.IsMouseButtonUp(MouseButton.Left);
			bool rightTriggerPressed = Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > 0f;

			if ((leftMouseButtonPressed || rightTriggerPressed) && waitTimeOver)
			{
				if (ammo == 0)
				{
					emptyGunSound.Play();
				}
				else
				{
					Shoot(bulletsPerShot);
				}
			}
			else if (leftMouseButtonReleased)
			{
				muzzleBlast.Stop();
			}
		}

		void UpdateState()
		{
			eyeLookDirection = eyes.transform.Forward;
			playerLookDirection = player.transform.Forward;
			muzzleBlast.Offset = eyeLookDirection + playerLookDirection;
		}

		void Shoot(uint bullets)
		{
			for (uint i = 0; i < bullets; i++)
			{
				CreateBullet();
			}

			gunshotSound.Play();
			muzzleBlast.Start();
			ammo--;
			timeToWait = timeBetweenShots;
		}

		void CreateBullet()
		{
			Entity bullet = new Entity("Bullet");
			bullet.transform.Translation = transform.Translation + transform.Forward;
			bullet.transform.Translation = 
			bullet.transform.Scale *= 0.5f;
			MeshRenderer meshRenderer = bullet.AddComponent<MeshRenderer>();
			meshRenderer.Type = MeshType.Sphere;
			bullet.AddComponent<BoxCollider>();
			RigidBody rb = bullet.AddComponent<RigidBody>();
			rb.BodyType = RigidBodyType.Dynamic;
			rb.AddForce(transform.Forward * bulletSpeed, ForceMode.Impulse);
		}
	}
}

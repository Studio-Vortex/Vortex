﻿using Sparky;

namespace Sandbox {

	public class Gun : Entity
	{
		public float timeBetweenShots = 1.4f;
		public float bulletSpeed = 180f;
		public float normalFOV = 60f;
		public float zoomedFOV = 40f;
		public uint startingAmmo = 10;

		private float timeToWait = 0f;
		private uint ammo = 0;
		private bool isZoomed = false;

		Vector3 startPosition;
		Vector3 startRotation;
		Vector3 zoomedPosition;
		Vector3 zoomedRotation;

		Camera camera;

		Entity player;
		Entity eyes;
		TextMesh ammoText;
		AudioSource gunshotSound;
		AudioSource emptyGunSound;
		AudioSource reloadSound;
		ParticleEmitter muzzleBlast;

		protected override void OnCreate()
		{
			camera = FindEntityByName("Camera").GetComponent<Camera>();
			player = FindEntityByName("Player");
			eyes = FindEntityByName("Eyes");
			emptyGunSound = FindEntityByName("Empty Gun Sound").GetComponent<AudioSource>();
			reloadSound = FindEntityByName("Reload Sound").GetComponent<AudioSource>();
			ammoText = FindEntityByName("Ammo Text").GetComponent<TextMesh>();
			gunshotSound = GetComponent<AudioSource>();
			muzzleBlast = GetComponent<ParticleEmitter>();
			startPosition = transform.Translation;
			startRotation = transform.Rotation;
			Transform zoomedTransform = FindEntityByName("Zoomed Transform").transform;

			zoomedPosition = zoomedTransform.Translation;
			zoomedRotation = zoomedTransform.Rotation;
			ammo = startingAmmo;
		}

		protected override void OnUpdate(float deltaTime)
		{
			ProcessFire();
			ProcessZoom();
			if (ammo == 0)
				Reload();

			ammoText.Text = ammo.ToString();

			timeToWait -= Time.DeltaTime;
		}

		public void AddAmmo(uint amount)
		{
			ammo += amount;
		}

		void ProcessZoom()
		{
			bool rightMouseButtonPressed = Input.IsMouseButtonDown(MouseButton.Right);
			bool leftTriggerPressed = Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > 0f;

			if (rightMouseButtonPressed || leftTriggerPressed)
			{
				camera.FieldOfView = zoomedFOV;
				transform.Translation = zoomedPosition;
				transform.Rotation = zoomedRotation;
				isZoomed = true;
			}
			else
			{
				camera.FieldOfView = normalFOV;
				transform.Translation = startPosition;
				transform.Rotation = startRotation;
				isZoomed = false;
			}
		}

		void ProcessFire()
		{
			bool leftMouseButtonPressed = Input.IsMouseButtonDown(MouseButton.Left);
			bool leftMouseButtonReleased = Input.IsMouseButtonUp(MouseButton.Left);
			bool rightTriggerPressed = Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > 0f;
			bool rightTriggerReleased = Input.GetGamepadAxis(Gamepad.AxisRightTrigger) == -1;
			bool waitTimeOver = timeToWait <= 0f;

			if ((leftMouseButtonPressed || rightTriggerPressed) && waitTimeOver)
			{
				if (ammo == 0)
				{
					emptyGunSound.Play();
				}
				else
				{
					Shoot();
				}
			}
			else if (leftMouseButtonReleased && rightTriggerReleased)
			{
				muzzleBlast.Stop();
			}
		}

		void Shoot()
		{
			Vector3 origin = isZoomed ? transform.worldTransform.Translation : player.transform.Translation + player.transform.Forward;
			if (Physics.Raycast(origin, eyes.transform.Forward, 200f, out RaycastHit hitInfo))
			{
				// TODO once we can get the entity from a raycast hit process it here
				if (hitInfo.Entity != null)
				{
					Debug.Log($"{hitInfo.Entity.Tag}");
				}

				/// Debug
				/*Entity collision = new Entity("Collision");
				collision.transform.Translation = hitInfo.Position;

				MeshRenderer meshRenderer = collision.AddComponent<MeshRenderer>();
				meshRenderer.Type = MeshType.Sphere;
				Material material = meshRenderer.GetMaterial();
				material.Albedo = hitInfo.Normal;*/
			}

			CreateBullet();

			gunshotSound.Play();
			muzzleBlast.Start();
			muzzleBlast.Offset = transform.Translation + transform.Forward;
			timeToWait = timeBetweenShots;
			ammo--;
		}

		void CreateBullet()
		{
			Entity bullet = new Entity("Bullet");
			bullet.transform.Translation = transform.worldTransform.Translation;
			bullet.transform.Scale *= 0.25f;

			MeshRenderer meshRenderer = bullet.AddComponent<MeshRenderer>();
			meshRenderer.Type = MeshType.Sphere;
			Material material = meshRenderer.GetMaterial();
			material.Albedo = Color.Red.XYZ;

			bullet.AddComponent<SphereCollider>();
			RigidBody rb = bullet.AddComponent<RigidBody>();
			rb.BodyType = RigidBodyType.Dynamic;
			rb.AddForce(transform.Forward * bulletSpeed, ForceMode.Impulse);
		}

		void Reload()
		{
			bool rKeyPressed = Input.IsKeyDown(KeyCode.R);
			bool yButtonPressed = Input.IsGamepadButtonDown(Gamepad.ButtonY);

			if (rKeyPressed || yButtonPressed)
			{
				reloadSound.Play();
				ammo = startingAmmo;
			}
		}
	}
}

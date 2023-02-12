using Vortex;

namespace Sandbox.Shooter.Weapons {

	using Shooter.Collections;
	using Shooter.AI;

	public class Gun : Entity
	{
		public float timeBetweenShots = 1.4f;
		public float bulletSpeed = 180f;
		public float normalFOV = 60f;
		public float zoomedFOV = 40f;
		public uint startingAmmo = 25;
		public float damageAmount;
		public uint weaponType;

		float timeToWait = 0f;
		uint ammo = 0;
		bool isZoomed = false;

		static BulletPool bulletPool;

		Vector3 startPosition;
		Vector3 startRotation;
		Vector3 zoomedPosition;
		Vector3 zoomedRotation;

		Camera camera;

		Entity player;
		Entity eyes;
		Entity ammoTextEntity;
		TextMesh ammoText;
		AudioSource gunshotSound;
		AudioSource emptyGunSound;
		AudioSource reloadSound;
		ParticleEmitter muzzleBlast;

		protected override void OnCreate()
		{
			camera = Scene.FindEntityByName("Camera").GetComponent<Camera>();
			player = Scene.FindEntityByName("Player");
			eyes = Scene.FindEntityByName("Eyes");
			emptyGunSound = Scene.FindEntityByName("Empty Gun Sound").GetComponent<AudioSource>();
			reloadSound = Scene.FindEntityByName("Reload Sound").GetComponent<AudioSource>();
			ammoTextEntity = Scene.FindEntityByName("Ammo Text");
			ammoText = ammoTextEntity.GetComponent<TextMesh>();
			gunshotSound = GetComponent<AudioSource>();
			muzzleBlast = GetComponent<ParticleEmitter>();

			WeaponType weapon = (WeaponType)weaponType;
			Transform zoomedTransform = new Transform();

			switch (weapon)
			{
				case WeaponType.Pistol: zoomedTransform = Scene.FindEntityByName("Pistol Zoomed Transform").transform; break;
				case WeaponType.Rifle:  zoomedTransform = Scene.FindEntityByName("Rifle Zoomed Transform").transform;  break;
			}

			startPosition = transform.Translation;
			startRotation = transform.Rotation;
			zoomedPosition = zoomedTransform.Translation;
			zoomedRotation = zoomedTransform.Rotation;
			ammo = startingAmmo;

			bulletPool = new BulletPool();
		}

		protected override void OnUpdate(float deltaTime)
		{
			ProcessFire();
			ProcessZoom();
			ReloadIfNeeded();
			UpdateAmmoTextAndTimeBetweenShots();
		}

		void UpdateAmmoTextAndTimeBetweenShots()
		{
			ammoText.Text = $"{ammo}/{startingAmmo}";
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
				camera.FieldOfView = Mathf.Lerp(zoomedFOV, normalFOV, Time.DeltaTime);
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
			else if (leftMouseButtonReleased || rightTriggerReleased)
			{
				muzzleBlast.Stop();
			}
		}

		void Shoot()
		{
			Vector3 origin = isZoomed ? transform.worldTransform.Translation : player.transform.Translation + player.transform.Forward;
			if (Physics.Raycast(origin, eyes.transform.Forward, 200f, out RaycastHit hitInfo))
			{
				switch (hitInfo.Entity.Marker)
				{
					case "Enemy":
					{
						Enemy enemy = hitInfo.Entity.As<Enemy>();
						enemy.OnEnemyHit(damageAmount);
						break;
					}
				}
			}

			UpdateOrCreateBullet();
			PlayEffects();
			UpdateWeaponState();
		}

		void UpdateOrCreateBullet()
		{
			if (bulletPool.IsFilled)
			{
				Entity bullet = bulletPool.Next();
				bullet.RemoveComponent<RigidBody>();
				bullet.transform.Translation = transform.worldTransform.Translation;
				RigidBody rb = bullet.AddComponent<RigidBody>();
				rb.BodyType = RigidBodyType.Dynamic;
				rb.AddForce(transform.Forward * bulletSpeed, ForceMode.Impulse);
			}
			else
			{
				Entity bullet = Scene.CreateEntity("Bullet");
				CreateBullet(bullet);
			}
		}

		void CreateBullet(Entity bullet)
		{
			bullet.transform.Translation = transform.worldTransform.Translation;
			bullet.transform.Scale *= 0.25f;

			MeshRenderer meshRenderer = bullet.AddComponent<MeshRenderer>();
			meshRenderer.Type = MeshType.Sphere;
			Material material = meshRenderer.GetSubmesh(0).Material;
			material.Albedo = Color.Red;

			bullet.AddComponent<SphereCollider>();
			RigidBody rb = bullet.AddComponent<RigidBody>();
			rb.BodyType = RigidBodyType.Dynamic;
			rb.AddForce(transform.Forward * bulletSpeed, ForceMode.Impulse);
			bulletPool.AddBullet(bullet);
		}

		void PlayEffects()
		{
			gunshotSound.Play();
			muzzleBlast.Start();
		}

		void UpdateWeaponState()
		{
			Vector3 forward = transform.Forward * 2.0f;
			muzzleBlast.Offset = forward;
			muzzleBlast.Velocity = forward;
			timeToWait = timeBetweenShots;
			ammo--;
		}

		void ReloadIfNeeded()
		{
			if (ammo == startingAmmo)
				return;

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

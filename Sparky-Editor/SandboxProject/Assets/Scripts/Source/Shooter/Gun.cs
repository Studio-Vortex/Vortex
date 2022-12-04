using Sparky;

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

		Vector3 startPosition;
		Vector3 startRotation;
		Vector3 zoomedPosition;
		Vector3 zoomedRotation;

		Camera camera;

		TextMesh ammoText;
		AudioSource gunshotSound;
		AudioSource emptyGunSound;
		AudioSource reloadSound;
		ParticleEmitter muzzleBlast;

		protected override void OnCreate()
		{
			camera = FindEntityByName("Camera").GetComponent<Camera>();
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
			ReloadIfNeeded();

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
					Shoot();
				}
			}
			else if (leftMouseButtonReleased)
			{
				muzzleBlast.Stop();
			}
		}

		void Shoot()
		{
			if (Physics.Raycast(transform.worldTransform.Translation, transform.Forward, 200f, out RaycastHit hitInfo))
			{
				// TODO once we can get the entity from a raycast hit process it here
				if (hitInfo.Entity != null)
				{
					Debug.Log($"{hitInfo.Entity.Tag}");
				}

				Entity collision = new Entity("Collision");
				collision.transform.Translation = hitInfo.Position;

				MeshRenderer meshRenderer = collision.AddComponent<MeshRenderer>();
				meshRenderer.Type = MeshType.Sphere;
				Material material = meshRenderer.GetMaterial();
				material.Albedo = hitInfo.Normal;
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

		void ReloadIfNeeded()
		{
			if (ammo != 0)
				return;

			if (Input.IsKeyDown(KeyCode.R))
			{
				reloadSound.Play();
				ammo = startingAmmo;
			}
		}
	}
}

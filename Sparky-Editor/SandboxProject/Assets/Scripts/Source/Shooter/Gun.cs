using Sparky;

namespace Sandbox {

	public class Gun : Entity
	{
		public float timeBetweenShots = 1.4f;
		public float bulletSpeed = 180f;
		public float normalFOV = 60f;
		public float zoomedFOV = 40f;
		public uint ammo = 10;

		float timeToWait = 0f;
		uint reloadAmount;

		Vector3 startPosition;
		Vector3 startRotation;
		Vector3 zoomedPosition;
		Vector3 zoomedRotation;

		Camera camera;

		AudioSource gunshotSound;
		AudioSource emptyGunSound;
		AudioSource reloadSound;
		ParticleEmitter muzzleBlast;

		protected override void OnCreate()
		{
			camera = FindEntityByName("Camera").GetComponent<Camera>();
			emptyGunSound = FindEntityByName("Empty Gun Sound").GetComponent<AudioSource>();
			reloadSound = FindEntityByName("Reload Sound").GetComponent<AudioSource>();
			gunshotSound = GetComponent<AudioSource>();
			muzzleBlast = GetComponent<ParticleEmitter>();
			startPosition = transform.Translation;
			startRotation = transform.Rotation;
			Transform zoomedTransform = FindEntityByName("Zoomed Transform").transform;
			zoomedPosition = zoomedTransform.Translation;
			zoomedRotation = zoomedTransform.Rotation;
			reloadAmount = ammo;
		}

		protected override void OnUpdate(float deltaTime)
		{
			ProcessFire();
			ProcessZoom();
			ReloadIfNeeded();

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
			if (Physics.Raycast(transform.Translation, transform.Forward, 100f, out RaycastHit hitInfo))
			{
				Vector3 normal = hitInfo.Normal;
				Vector3 position = hitInfo.Position;
				float distance = hitInfo.Distance;

				Debug.Log($"UUID: {hitInfo.EntityID}");

				if (hitInfo.Entity != null)
				{
					Entity entity = hitInfo.Entity;
					Debug.Log($"Hit! N: {normal}, P: {position}, D: {distance}, Name: {entity.Tag}");
				}
			}

			CreateBullet();

			gunshotSound.Play();
			muzzleBlast.Start();
			muzzleBlast.Offset = transform.Translation + transform.Forward;
			ammo--;
			timeToWait = timeBetweenShots;
		}

		void CreateBullet()
		{
			Entity bullet = new Entity("Bullet");
			bullet.transform.Translation = transform.worldTransform.Translation;
			bullet.transform.Scale *= 0.5f;

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
				ammo = reloadAmount;
			}
		}
	}
}

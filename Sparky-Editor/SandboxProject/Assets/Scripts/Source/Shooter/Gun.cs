using Sparky;

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

		Camera camera;

		AudioSource gunshotSound;
		AudioSource emptyGunSound;
		ParticleEmitter muzzleBlast;

		protected override void OnCreate()
		{
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
			muzzleBlast.Offset = transform.Translation + transform.Forward;
		}

		void Shoot(uint bullets)
		{
			for (uint i = 0; i < bullets; i++)
			{
				CreateBullet();
			}

			if (Physics.Raycast(transform.Translation, transform.Forward, 100f, out RaycastHit hitInfo))
			{
				Vector3 normal = hitInfo.Normal;
				Vector3 position = hitInfo.Position;
				float distance = hitInfo.Distance;
				if (hitInfo.Entity != null)
				{
					Entity entity = hitInfo.Entity;
					Debug.Log($"Hit! N: {normal}, P: {position}, D: {distance}, Name: {entity.Tag}");
				}
			}

			gunshotSound.Play();
			muzzleBlast.Start();
			ammo--;
			timeToWait = timeBetweenShots;
		}

		void CreateBullet()
		{
			Entity bullet = new Entity("Bullet");
			bullet.transform.Translation = transform.worldTransform.Translation + transform.Forward;
			Debug.Warn(transform.Forward.ToString());
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
	}
}

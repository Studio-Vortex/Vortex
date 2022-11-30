using Sparky;

namespace Sandbox {

	public class Gun : Entity
	{
		public float timeBetweenShots = 2f;
		public float bulletSpeed = 10.0f;

		float timeToWait = 0f;

		Vector3 eyeLookDirection;
		Entity eyes;
		Vector3 playerLookDirection;
		Entity player;

		Camera camera;

		AudioSource gunshotSound;
		ParticleEmitter muzzleBlast;

		protected override void OnCreate()
		{
			eyes = FindEntityByName("Eyes");
			player = FindEntityByName("Player");
			camera = FindEntityByName("Camera").GetComponent<Camera>();
			gunshotSound = GetComponent<AudioSource>();
			muzzleBlast = GetComponent<ParticleEmitter>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			UpdateState();
			ProcessFire();
			ProcessZoom();

			timeToWait -= Time.DeltaTime;
		}

		private static void ProcessZoom()
		{
			bool rightMouseButtonPressed = Input.IsMouseButtonDown(MouseButton.Right);
			bool rightMouseButtonReleased = Input.IsMouseButtonUp(MouseButton.Right);
			bool leftTriggerPressed = Input.GetGamepadAxis(Gamepad.AxisLeftTrigger) > 0f;

			if (rightMouseButtonPressed || leftTriggerPressed)
			{
				
			}
			else if (rightMouseButtonReleased)
			{

			}
		}

		private void ProcessFire()
		{
			bool waitTimeOver = timeToWait <= 0f;
			bool leftMouseButtonPressed = Input.IsMouseButtonDown(MouseButton.Left);
			bool leftMouseButtonReleased = Input.IsMouseButtonUp(MouseButton.Left);
			bool rightTriggerPressed = Input.GetGamepadAxis(Gamepad.AxisRightTrigger) > 0f;

			if ((leftMouseButtonPressed || rightTriggerPressed) && waitTimeOver)
			{
				Fire(1);
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

		void Fire(uint bullets)
		{
			for (uint i = 0; i < bullets; i++)
			{
				CreateBullet();
			}

			gunshotSound.Play();
			muzzleBlast.Start();
			timeToWait = timeBetweenShots;
		}

		void CreateBullet()
		{
			Entity bullet = new Entity("Bullet");
			bullet.transform.Translation = player.transform.Translation + player.transform.Forward;
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

using Vortex;

namespace Sandbox {

	public class Driver : Entity
	{
		public float moveSpeed;
		public float steerSpeed;
		public float slowSpeed;
		public float boostSpeed;
		public bool hasPackage;
		public Color4 hasPackageColor = new Color4(0.2f, 0.8f, 0.2f, 1.0f);
		public Color4 noPackageColor = new Color4(1.0f, 1.0f, 1.0f, 1.0f);

		Entity camera;
		SpriteRenderer spriteRenderer;
		TextMesh packagesLeftText;

		protected override void OnCreate()
		{
			spriteRenderer = GetComponent<SpriteRenderer>();
			packagesLeftText = Scene.FindEntityByName("Text").GetComponent<TextMesh>();
			camera = Scene.FindEntityByName("Camera");
		}

		protected override void OnUpdate(float delta)
		{
			ProcessRotation();
			ProcessMovement();
			
			UpdateCamera();
		}

		protected override void OnCollisionEnter(Collision other)
		{
			HandleCollision(other);
		}

		protected override void OnTriggerEnter(Collision other)
		{
			HandleTrigger(other);
		}

		void HandleCollision(Collision other)
		{
			ProcessDelivery(other);
		}

		void HandleTrigger(Collision other)
		{
			ProcessDelivery(other);
		}

		void ProcessRotation()
		{
			float steerAmount = 0.0f;

			if (Input.IsKeyDown(KeyCode.A))
				steerAmount = -Mathf.Abs(steerSpeed);
			else if (Input.IsKeyDown(KeyCode.D))
				steerAmount = Mathf.Abs(steerSpeed);

			transform.Rotate(Vector3.Forward * steerAmount * Time.DeltaTime);
		}

		void ProcessMovement()
		{
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.W))
				velocity = transform.Up;
			else if (Input.IsKeyDown(KeyCode.S))
				velocity = -transform.Up;

			transform.Translate(velocity * moveSpeed * Time.DeltaTime);
		}

		void ProcessDelivery(Collision other)
		{
			Entity entity = other.Entity;

			switch (entity.Marker)
			{
				case "Package":
					if (!hasPackage)
					{
						hasPackage = true;
						spriteRenderer.Color = hasPackageColor;
						Destroy(entity);
					}

					break;
				case "Customer":
					if (hasPackage)
					{
						hasPackage = false;
						spriteRenderer.Color = noPackageColor;
						int packagesLeft = int.Parse(packagesLeftText.Text);
						packagesLeftText.Text = (--packagesLeft).ToString();
					}

					break;
				case "Boost":
					moveSpeed = boostSpeed;
					Destroy(entity);
					break;
				case "UnTagged":
					moveSpeed = slowSpeed;
					break;
			}
		}

		void UpdateCamera()
		{
			camera.transform.Translation = transform.Translation;
		}

	}

}

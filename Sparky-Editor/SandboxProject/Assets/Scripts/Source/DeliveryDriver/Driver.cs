using Sparky;
using System;

namespace Sandbox {

	public class Driver : Entity
	{
		public float moveSpeed;
		public float steerSpeed;
		public float slowSpeed;
		public float boostSpeed;
		public bool hasPackage;

		Vector4 hasPackageColor = new Vector4(0.2f, 0.8f, 0.2f, 1.0f);
		Vector4 noPackageColor = new Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		SpriteRenderer spriteRenderer;
		TextMesh packagesLeftText;

		protected override void OnCreate()
		{
			spriteRenderer = GetComponent<SpriteRenderer>();
			packagesLeftText = FindEntityByName("Text").GetComponent<TextMesh>();
		}

		protected override void OnUpdate(float delta)
		{
			ProcessRotation();
			ProcessMovement();
			ProcessDelivery();
		}

		void ProcessRotation()
		{
			float steerAmount = 0.0f;

			if (Input.IsKeyDown(KeyCode.A))
				steerAmount = -Math.Abs(steerSpeed);
			else if (Input.IsKeyDown(KeyCode.D))
				steerAmount = Math.Abs(steerSpeed);

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

		void ProcessDelivery()
		{
			Entity other = Physics2D.Raycast(transform.Translation.XY, transform.Translation.XY + transform.Up.XY, out RayCastHit2D hit);
			
			if (other.Marker == "Package" && !hasPackage)
			{
				hasPackage = true;
				spriteRenderer.Color = hasPackageColor;
				other.Destroy(false);
			}
			else if (other.Marker == "Customer" && hasPackage)
			{
				hasPackage = false;
				spriteRenderer.Color = noPackageColor;
				int packagesLeft = Convert.ToInt32(packagesLeftText.Text);
				packagesLeftText.Text = (--packagesLeft).ToString();
			}
			else if (other.Marker == "Boost")
			{
				moveSpeed = boostSpeed;
				other.Destroy(false);
			}
			else if (other.Marker == "UnTagged")
			{
				moveSpeed = slowSpeed;
			}
		}

	}

}

using Sparky;
using System;

namespace Sandbox {

	public class MarbleController : Entity
	{
		public float speed;

		/// <summary>
		/// OnCreate is called once when the Entity that this script is attached to
		/// is instantiated in the scene at runtime
		/// </summary>
		protected override void OnCreate()
		{

		}

		/// <summary>
		/// OnUpdate is called once every frame while this script is active in the scene
		/// </summary>
		protected override void OnUpdate(float deltaTime)
		{
			ProcessRotation();
			ProcessMovement();
		}

		void ProcessMovement()
		{
			Vector3 velocity = Vector3.Zero;

			if (Input.IsKeyDown(KeyCode.A))
				velocity = Vector3.Left;
			else if (Input.IsKeyDown(KeyCode.D))
				velocity = Vector3.Right;

			transform.Translate(velocity * speed * Time.DeltaTime);
		}

		void ProcessRotation()
		{
			Entity entity = FindEntityByName("Point Light");

			if (Input.IsKeyDown(KeyCode.Space))
			{
				if (entity != null)
				{
					entity.Destroy();
				}
			}

			if (entity == null && Input.IsKeyDown(KeyCode.P))
			{
				Entity newDirLight = new Entity("Directional Light");
				LightSource lightSource = newDirLight.AddComponent<LightSource>();
				newDirLight.transform.Translation = Vector3.Up * 3.0f;
				lightSource.Direction = Vector3.Down;
			}
		}
	}

}

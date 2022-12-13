using Sparky;

namespace Sandbox.Facerun {

	public class Controller : Entity
	{
		public Vector2 finishPoint;

		Vector3 startColor;
		Material material;
		Entity text;
		RigidBody rb;
		bool firstClick = false;

		protected override void OnCreate()
		{
			rb = GetComponent<RigidBody>();
			text = FindEntityByName("Game Over Text");
			material = GetComponent<MeshRenderer>().GetMaterial();
			startColor = material.Albedo;
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Up) && !firstClick)
			{
				rb.Translate(Vector3.Up);
				firstClick = true;
			}
			if (Input.IsKeyDown(KeyCode.Down) && !firstClick)
			{
				rb.Translate(Vector3.Down);
				firstClick = true;
			}
			if (Input.IsKeyDown(KeyCode.Left) && !firstClick)
			{
				rb.Translate(Vector3.Left);
				firstClick = true;
			}
			if (Input.IsKeyDown(KeyCode.Right) && !firstClick)
			{
				rb.Translate(Vector3.Right);
				firstClick = true;
			}

			bool allKeysReleased = Input.IsKeyUp(KeyCode.Up) && Input.IsKeyUp(KeyCode.Down) && Input.IsKeyUp(KeyCode.Left) && Input.IsKeyUp(KeyCode.Right);
			if (firstClick && allKeysReleased)
				firstClick = false;

			if (transform.Translation.XY == finishPoint)
			{
				text.SetActive(true);
				material.Albedo = Color.Green.XYZ;
			}
			else
			{
				text.SetActive(false);
				//material.Albedo = startColor;
			}
		}
	}

}

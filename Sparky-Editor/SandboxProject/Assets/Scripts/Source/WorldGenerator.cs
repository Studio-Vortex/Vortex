using System;
using Sparky;

namespace Sandbox {

	public class WorldGenerator : Entity
	{
		public int Rows;
		public int Columns;
		public float Offset;
		public float Step;

		protected override void OnCreate()
		{
			// Called once before the first frame
			for (float y = 0; y < Rows; y += Step)
			{
				for (float x = 0; x < Columns; x += Step)
				{
					Entity entity = new Entity($"{x}, {y}");
					entity.transform.Translation = new Vector3(x + Offset, y + Offset, 0.0f);
					
					SpriteRenderer spriteRenderer = entity.AddComponent<SpriteRenderer>();
					spriteRenderer.Color = new Vector4(
						RandomDevice.RangedFloat(0.0f, 1.0f), RandomDevice.RangedFloat(0.0f, 1.0f), RandomDevice.RangedFloat(0.0f, 1.0f), 1.0f
					);

					RigidBody2D rigidBody = entity.AddComponent<RigidBody2D>();
					//rigidBody.Type = RigidBody2D.BodyType.Static;
					BoxCollider2D boxCollider = entity.AddComponent<BoxCollider2D>();
				}
			}
		}

		protected override void OnUpdate(float delta)
		{
			// Called once every frame
		}
	}

}
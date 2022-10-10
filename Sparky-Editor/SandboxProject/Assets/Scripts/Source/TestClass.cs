using System;
using Sparky;

namespace Sandbox {

	public class TestClass : Entity
	{
		public Entity entity;
		public SpriteRenderer spriteRenderer;

		public override void OnCreate()
		{
			spriteRenderer = entity.GetComponent<SpriteRenderer>();
		}

		public override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Space))
			{
				spriteRenderer.Color = new Vector4(0.2f, 0.2f, 0.8f, 1.0f);
			}
		}
	}

}

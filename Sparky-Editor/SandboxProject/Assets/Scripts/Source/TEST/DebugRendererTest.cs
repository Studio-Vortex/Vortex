using Sparky;

namespace Sandbox {

	public class DebugRendererTest : Entity
	{
		public int size;

		protected override void OnUpdate(float deltaTime)
		{
			DebugRenderer.BeginScene();

			for (int z = -size; z < size; z += 2)
			{
				for (int y = -size; y < size; y += 2)
				{
					for (int x = -size; x < size; x += 2)
					{
						Vector3 point = new Vector3(x, y, z) + (size * 0.25f);
						DebugRenderer.DrawLine(Vector3.Zero, point, Color.Yellow);
						DebugRenderer.DrawQuadBillboard(point, new Vector2(1), Color.Blue);
					}
				}
			}

			DebugRenderer.Flush();
		}
	}

}
using Vortex;

namespace Sandbox.Basketball {

	public class DestroyOnClick : Entity
	{
		protected override void OnUpdate(float delta)
		{
			if (Input.IsMouseButtonDown(MouseButton.Left))
			{
				Entity hoveredEntity = Scene.GetHoveredEntity();
				if (hoveredEntity.Tag == "Removable Cube")
				{
					Destroy(hoveredEntity);
				}
			}
		}
	}

}

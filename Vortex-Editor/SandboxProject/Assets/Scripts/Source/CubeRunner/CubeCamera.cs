using Vortex;

namespace Sandbox {

	public class CubeCamera : Entity
	{
		public Vector3 offset;

		Entity player;

		protected override void OnCreate()
		{
			player = Scene.FindEntityByName("Player");
		}

		protected override void OnUpdate(float deltaTime)
		{
			transform.Translation = player.transform.Translation + offset;
		}
	}

}

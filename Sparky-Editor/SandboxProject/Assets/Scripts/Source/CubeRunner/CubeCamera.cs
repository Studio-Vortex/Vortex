using Sparky;

namespace Sandbox {

	public class CubeCamera : Entity
	{
		public Vector3 offset;

		Entity player;

		protected override void OnCreate()
		{
			player = FindEntityByName("Player");
		}

		protected override void OnUpdate(float deltaTime)
		{
			transform.Translation = player.transform.Translation + offset;
		}
	}

}

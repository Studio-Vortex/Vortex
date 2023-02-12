using Vortex;

namespace Sandbox {

	public class LookAtTest : Entity
	{
		Entity player;

		protected override void OnCreate()
		{
			player = Scene.FindEntityByName("Player");
		}

		protected override void OnUpdate(float delta)
		{
			transform.LookAt(player.transform.Translation);
		}
	}

}

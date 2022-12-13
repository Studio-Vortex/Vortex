using Vortex;

namespace Sandbox {

	public class LookAtTest : Entity
	{
		Entity player;

		protected override void OnCreate()
		{
			player = FindEntityByName("Player");
		}

		protected override void OnUpdate(float delta)
		{
			transform.LookAt(player.transform.Translation);
		}
	}

}

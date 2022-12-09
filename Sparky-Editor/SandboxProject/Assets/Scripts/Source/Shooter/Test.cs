using Sparky;

namespace Sandbox.Shooter.AI {

	public class Test : Entity
	{
		Entity player;

		void OnCreate()
		{
			player = FindEntityByName("Player");
		}

		void OnUpdate(float delta)
		{
			transform.LookAt(player.transform.Translation);
		}
	}

}

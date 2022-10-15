using System;
using Sparky;

namespace Sandbox {

	public class Music : Entity
	{
		public PlatformerPlayer player;
		public bool IsOver = false;

		public override void OnCreate()
		{
			player = FindEntityByName("Player").As<PlatformerPlayer>();
		}

		public override void OnUpdate(float delta)
		{
			if (IsOver)
				return;

			if (player.GameOver)
			{
				IsOver = true;
				GetComponent<AudioSource>().Play();
			}
		}
	}

}

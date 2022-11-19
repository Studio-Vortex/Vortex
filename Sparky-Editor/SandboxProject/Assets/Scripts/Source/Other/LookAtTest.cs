using System;
using Sparky;

namespace Sandbox {

	public class LookAtTest : Entity
	{
		public Entity player;
		public Vector3 StartPos;

		protected override void OnCreate()
		{
			player = FindEntityByName("Player");
			StartPos = transform.Translation;
		}

		protected override void OnUpdate(float delta)
		{
			transform.Translation = StartPos;
			transform.LookAt(player.transform.Translation);
		}
	}

}

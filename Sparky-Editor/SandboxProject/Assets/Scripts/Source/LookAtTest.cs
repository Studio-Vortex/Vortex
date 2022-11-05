using System;
using Sparky;

namespace Sandbox {

	public class LookAtTest : Entity
	{
		public Entity player;
		public Vector3 StartPos;

		public override void OnCreate()
		{
			player = FindEntityByName("Player");
			StartPos = transform.Translation;
		}

		public override void OnUpdate(float delta)
		{
			transform.Translation = StartPos;
			transform.LookAt(player.transform.Translation);
		}
	}

}

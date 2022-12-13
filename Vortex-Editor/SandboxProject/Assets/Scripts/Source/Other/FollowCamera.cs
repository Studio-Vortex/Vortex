using Vortex;
using System;

namespace Sandbox {

	public class FollowCamera : Entity
	{
		public Vector3 offset;
		private Entity m_player;

		protected override void OnCreate()
		{
			m_player = FindEntityByName("Rocket");
		}

		protected override void OnUpdate(float deltaTime)
		{
			transform.Translation = m_player.transform.Translation + offset;
		}
	}

}

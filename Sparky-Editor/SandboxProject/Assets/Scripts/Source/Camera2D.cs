﻿using System;
using Sparky;

namespace Sandbox {

	public class Camera2D : Entity
	{
		public float DistanceToPlayer;
		private Entity m_Player;

		public void OnCreate()
		{
			m_Player = FindEntityByName("Player");
		}

		public void OnUpdate(float delta)
		{
			if (m_Player != null)
				transform.Translation = new Vector3(m_Player.transform.Translation.XY, DistanceToPlayer);
		}
	}

}

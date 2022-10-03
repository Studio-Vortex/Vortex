﻿using System;
using Sparky;

namespace Sandbox {

	public class MovingWall : Entity
	{
		public float Bounds;
		public float Speed;
		private Vector3 m_Velocity;
		private bool m_Left;

		public override void OnCreate()
		{
			m_Left = true;
		}

		public override void OnUpdate(float delta)
		{
			if (m_Left)
			{
				m_Velocity.X = -1.0f;
				if (transform.Translation.X <= -Bounds)
					m_Left = false;

			}
			else
			{
				m_Velocity.X = 1.0f;
				if (transform.Translation.X >= Bounds)
					m_Left = true;
			}

			m_Velocity *= Speed * delta;

			transform.Translation += m_Velocity;
		}

		public override void OnDestroy()
		{
			Debug.Log("MovingWall OnDestroy() Called!");
		}
	}

}

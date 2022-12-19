﻿using System;
using Vortex;

namespace Sandbox {

	public class Rotate : Entity
	{
		public float Speed;

		protected override void OnCreate()
		{
			Debug.Log("Hello from C#!");
		}

		protected override void OnUpdate(float delta)
		{
			transform.Rotate(0.0f, Speed * delta, 0.0f);
		}
	}

}
using System;
using Sparky;

namespace Sandbox {

	public class Rotate : Entity
	{
		public float Speed;

		public override void OnCreate()
		{
			Debug.Log("Hello from C#!");
		}

		public override void OnUpdate(float delta)
		{
			transform.Rotate(0.0f, Speed * delta, 0.0f);
		}
	}

}

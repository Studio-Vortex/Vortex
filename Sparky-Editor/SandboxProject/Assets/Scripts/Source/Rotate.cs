using System;
using Sparky;

namespace Sandbox {

	public class Rotate : Entity
	{
		public float Speed;

		public override void OnUpdate(float delta)
		{
			float speed = Speed * delta;

			transform.Rotate(speed, speed, speed);
		}
	}

}

using Sparky;
using System;

namespace Sandbox {

	public class Oscillator : Entity
	{
		public Vector3 Movement;
		public float MovementFactor;
		private Vector3 StartPosition;

		public override void OnCreate()
		{
			StartPosition = transform.Translation;
		}

		public override void OnUpdate(float deltaTime)
		{
			const float tau = Mathf.PI * 2.0f;
			Vector3 offset = Movement * MovementFactor;
			transform.Translation = StartPosition + offset;
		}
	}

}

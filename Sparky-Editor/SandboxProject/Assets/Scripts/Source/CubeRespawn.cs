using System;
using Sparky;

namespace Sandbox {

	public class CubeRespawn : Entity
	{
		public Vector3 StartPosition;
		public float ResetPos = -25.0f;

		public override void OnCreate()
		{
			StartPosition = transform.Translation;
		}

		public override void OnUpdate(float delta)
		{
			if (transform.Translation.Y <= ResetPos)
				transform.Translation = StartPosition;
		}
	}

}

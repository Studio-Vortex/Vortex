using Sparky;
using System;

namespace Sandbox {

	public class PhysicsTest : Entity
	{
		public float speed;
		private RigidBody rb;

		protected override void OnCreate()
		{
			rb = GetComponent<RigidBody>();
		}

		protected override void OnUpdate(float delta)
		{
			rb.ApplyTorque(Vector3.Forward * speed * delta);
		}
	}

}

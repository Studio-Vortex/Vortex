using Sparky;

namespace Sandbox.DarkRoom {

	public class OpenDoor : Entity
	{
		public float rotationSpeed;
		public float stopAngle;

		protected override void OnCreate()
		{
			
		}

		protected override void OnUpdate(float delta)
		{
			if (transform.Rotation.Y < stopAngle)
			{
				transform.Rotate(0, rotationSpeed * delta, 0);
			}
		}
	}

}

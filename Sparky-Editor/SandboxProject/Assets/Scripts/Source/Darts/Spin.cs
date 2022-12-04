using Sparky;

namespace Sandbox {

	public class Spin : Entity
	{
		public float rotationSpeed = 35;

		protected override void OnUpdate(float deltaTime)
		{
			transform.Rotate(0, 0, rotationSpeed * Time.DeltaTime);
		}
	}

}

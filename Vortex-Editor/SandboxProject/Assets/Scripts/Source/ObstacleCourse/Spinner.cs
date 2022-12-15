using Vortex;

namespace Sandbox.ObstacleCourse
{
	public class Spinner : Entity
	{
		public float spinSpeed = 5f;

		protected override void OnUpdate(float deltaTime)
		{
			transform.Rotate(0, spinSpeed, 0);
		}
	}
}

using Sparky;

namespace Sandbox {
	
	public class ScrollingText : Entity
	{
		public float speed;

		protected override void OnUpdate(float delta)
		{
			transform.Translate(0, speed * delta, 0);
		}
	}

}

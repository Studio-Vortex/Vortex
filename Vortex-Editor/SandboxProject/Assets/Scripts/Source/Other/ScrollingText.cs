using Vortex;

namespace Sandbox {
	
	public class ScrollingText : Entity
	{
		public float speed;

		protected override void OnUpdate(float delta)
		{
			transform.Translate(transform.Up * speed * delta);
		}
	}

}

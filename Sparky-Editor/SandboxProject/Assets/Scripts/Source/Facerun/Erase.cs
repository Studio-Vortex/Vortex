using Sparky;

namespace Sandbox {

	public class Erase : Entity
	{
		protected override void OnCreate()
		{
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Space))
			{
				Destroy();
			}
		}
	}

}

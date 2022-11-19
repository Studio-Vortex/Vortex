using Sparky;

namespace Sandbox {

	public class TopDownCamera : Entity
	{
		private Entity driver;

		protected override void OnCreate()
		{
			driver = FindEntityByName("Driver");
		}

		protected override void OnUpdate(float deltaTime)
		{
			transform.Translation = driver.transform.Translation;
		}
	}

}

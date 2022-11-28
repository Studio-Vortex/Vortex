using Sparky;

namespace Sandbox {

	public class NewClass : Entity
	{
		protected override void OnCreate()
		{
			
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Space))
			{
				SceneManager.LoadScene("FPSDemo");
			}
		}
	}

}

using Sparky;

namespace Sandbox {

	public class MenuTest : Entity
	{
		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.E))
			{
				SceneManager.LoadScene("Sponza");
			}
		}
	}

}

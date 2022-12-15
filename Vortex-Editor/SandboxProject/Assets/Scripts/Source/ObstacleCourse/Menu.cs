using Vortex;

namespace Sandbox.ObstacleCourse {

	public class Menu : Entity
	{
		protected override void OnUpdate(float deltaTime)
		{
			if (Input.IsKeyDown(KeyCode.Enter))
			{
				SceneManager.LoadScene("ObstacleCourse");
			}
		}
	}

}

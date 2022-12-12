using Sparky;

namespace Sandbox.TestGame {

	public class Test : Entity
	{
		protected override void OnCreate()
		{
			
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Space))
			{
				transform.SetParent(FindEntityByName("Cube"));
			}
			if (Input.IsKeyDown(KeyCode.R))
			{
				Scene.Instantiate(this).transform.Translation = new Vector3(-2, 3, -3);
			}
		}
	}

}

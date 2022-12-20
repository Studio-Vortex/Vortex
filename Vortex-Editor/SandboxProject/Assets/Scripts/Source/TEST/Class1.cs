using Vortex;

namespace Sandbox.RendererTest {

	public class Class1 : Entity
	{
		void OnCreate()
		{
			Debug.Log(transform.Forward);
		}

		void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W))
				transform.Translate(transform.Forward);
			if (Input.IsKeyDown(KeyCode.S))
				transform.Translate(-transform.Forward);
			if (Input.IsKeyDown(KeyCode.A))
				transform.Translate(-transform.Right);
			if (Input.IsKeyDown(KeyCode.D))
				transform.Translate(transform.Right);
		}
	}

}

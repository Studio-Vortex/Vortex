using Sparky;

namespace Sandbox {

	public class MonoBehaviour : Entity
	{
		protected override void OnCreate()
		{
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (Input.IsKeyDown(KeyCode.W))
			{
				transform.Translate(Vector3.Forward * deltaTime);
			}

			if (Input.IsMouseButtonDown(MouseButton.Left))
			{

			}
		}
	}

}

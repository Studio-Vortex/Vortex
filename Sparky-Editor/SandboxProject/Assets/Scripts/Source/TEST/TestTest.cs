using Sparky;

namespace Sandbox {

	public class TestTest : Entity
	{
		protected override void OnUpdate(float delta)
		{
			Vector2 scrollDelta = Input.GetMouseScrollDelta();
			Debug.Log($"{scrollDelta}");
		}
	}

}

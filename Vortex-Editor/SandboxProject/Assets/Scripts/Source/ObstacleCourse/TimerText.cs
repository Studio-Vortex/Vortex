using Vortex;

namespace Sandbox.ObstacleCourse {

	public class TimerText : Entity
	{
		float time = 30f;
		TextMesh textMesh;

		protected override void OnCreate()
		{
			textMesh = GetComponent<TextMesh>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			time -= deltaTime;
			textMesh.Text = time.ToString("0.0s");
		}
	}

}

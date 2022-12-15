using Vortex;

namespace Sandbox.ObstacleCourse {

	public class ScoreText : Entity
	{
		uint score = 0;
		uint scoreLastFrame = 0;
		TextMesh textMesh;

		protected override void OnCreate()
		{
			textMesh = GetComponent<TextMesh>();
		}

		protected override void OnUpdate(float deltaTime)
		{
			if (scoreLastFrame == score)
				return;

			textMesh.Text = $"Score: {score}";
			scoreLastFrame = score;
		}

		public void IncrementScore()
		{
			score++;
		}
	}

}

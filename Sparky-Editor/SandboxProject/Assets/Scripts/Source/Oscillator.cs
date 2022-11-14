using Sparky;

namespace Sandbox {

	public class Oscillator : Entity
	{
		public Vector3 Movement;
		public float MovementFactor;
		public float Period = 2f;
		private Vector3 StartPosition;

		public override void OnCreate()
		{
			StartPosition = transform.Translation;
		}

		public override void OnUpdate(float deltaTime)
		{
			float cycles = Time.Elapsed / Period;

			float tau = Mathf.PI * 2f;
			float rawSinWave = Mathf.Sin(cycles * tau);

			MovementFactor = rawSinWave * 0.5f + 0.5f;

			Vector3 offset = Movement * MovementFactor;
			transform.Translation = StartPosition + offset;
		}
	}

}

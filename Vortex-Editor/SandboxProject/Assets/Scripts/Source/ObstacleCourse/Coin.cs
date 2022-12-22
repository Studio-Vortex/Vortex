using Vortex;

namespace Sandbox.ObstacleCourse {

	public class Coin : Entity
	{
		float m_SpinSpeed = 2.5f;

		protected override void OnUpdate(float deltaTime)
		{
			transform.Rotate(0, m_SpinSpeed, 0);
		}

		protected override void OnRaycastCollision()
		{
			ScoreText scoreText = FindEntityByName("Score Text").As<ScoreText>();
			scoreText.IncrementScore();
			Destroy();

			if (FindEntityByName("Pickup Sound").TryGetComponent(out AudioSource pickupSound))
			{
				pickupSound.Play();
			}
		}
	}

}

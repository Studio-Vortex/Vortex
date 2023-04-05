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
			ScoreText scoreText = Scene.FindEntityByName("Score Text").As<ScoreText>();
			scoreText.IncrementScore();
			Destroy(this);

			if (Scene.FindEntityByName("Pickup Sound").TryGetComponent(out AudioSource pickupSound))
			{
				pickupSound.Play();
			}
		}
	}

}

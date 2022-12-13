using Vortex;

namespace Sandbox {

	public class TopDownCamera : Entity
	{
		Entity driver;
		AudioSource audioSource;

		protected override void OnCreate()
		{
			driver = FindEntityByName("Driver");
			audioSource = GetComponent<AudioSource>();
			audioSource.Play();
		}

		protected override void OnUpdate(float deltaTime)
		{
			transform.Translation = driver.transform.Translation;
		}
	}

}

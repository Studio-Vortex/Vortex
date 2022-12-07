using Sparky;

namespace Sandbox {

	public class MenuTest : Entity
	{
		Entity camera;
		AudioSource clickedSound;

		public float animationTime;
		float time;
		bool started = false;

		protected override void OnCreate()
		{
			camera = FindEntityByName("Camera");
			clickedSound = GetComponent<AudioSource>();
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.Enter) && !started)
			{
				started = true;
				time = 0.0f;
				clickedSound.Play();
			}

			if (started && time <= animationTime)
			{
				time += delta;
				camera.transform.Translate(Vector3.Forward * 2.0f * delta);
				if (time > animationTime)
				{
					SceneManager.LoadScene("FPSDemoRevamped");
				}
			}
		}
	}

}

using Sparky;

namespace Sandbox {

	public class MenuManager : Entity
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
			bool enterKeyPressed = Input.IsKeyDown(KeyCode.Enter);
			bool startButtonPressed = Input.IsGamepadButtonDown(Gamepad.ButtonStart);

			if (enterKeyPressed || startButtonPressed && !started)
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
					SceneManager.LoadScene("Shooter - Level01");
				}
			}
		}
	}

}

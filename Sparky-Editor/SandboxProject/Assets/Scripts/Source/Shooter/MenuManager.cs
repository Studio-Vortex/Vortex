using Sparky;

namespace Sandbox.Shooter.Menu {

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
			OnStartGame();
			UpdateAnimation();
		}

		private void OnStartGame()
		{
			bool enterKeyPressed = Input.IsKeyDown(KeyCode.Enter);
			bool startButtonPressed = Input.IsGamepadButtonDown(Gamepad.ButtonStart);

			if (enterKeyPressed || startButtonPressed && !started)
			{
				started = true;
				time = 0.0f;
				clickedSound.Play();
			}
		}

		void UpdateAnimation()
		{
			if (started && time <= animationTime)
			{
				time += Time.DeltaTime;
				camera.transform.Translate(Vector3.Forward * 2.0f * Time.DeltaTime);
				if (time > animationTime)
				{
					SceneManager.LoadScene("Shooter - Level01");
				}
			}
		}
	}

}

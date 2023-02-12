using Vortex;

namespace Sandbox.Shooter.Menu {

	public class MenuManager : Entity
	{
		public float animationTime;

		Entity camera;
		AudioSource clickedSound;
		float time;
		bool started = false;

		protected override void OnCreate()
		{
			camera = Scene.FindEntityByName("Camera");
			clickedSound = GetComponent<AudioSource>();
		}

		protected override void OnUpdate(float delta)
		{
			OnGameStart();
			UpdateAnimation();
		}

		void OnGameStart()
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

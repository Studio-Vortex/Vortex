using Sparky;

namespace Sandbox {

	public class WeaponManager : Entity
	{
		Entity rifle;
		Entity pistol;

		AudioSource weaponEquipedSound;

		protected override void OnCreate()
		{
			rifle = FindEntityByName("Rifle");
			pistol = FindEntityByName("Pistol");
			weaponEquipedSound = GetComponent<AudioSource>();
		}

		protected override void OnUpdate(float delta)
		{
			Vector2 scrollDelta = Input.GetMouseScrollDelta();

			if (scrollDelta.Y > 0 || Input.IsGamepadButtonDown(Gamepad.Up))
			{
				pistol.SetActive(true);
				rifle.SetActive(false);
				weaponEquipedSound.Play();
			}
			else if (scrollDelta.Y < 0 || Input.IsGamepadButtonDown(Gamepad.Down))
			{
				rifle.SetActive(true);
				pistol.SetActive(false);
				weaponEquipedSound.Play();
			}
		}
	}

}

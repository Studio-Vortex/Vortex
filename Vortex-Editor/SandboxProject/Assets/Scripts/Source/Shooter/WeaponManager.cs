using Vortex;

namespace Sandbox.Shooter.Weapons {

	public enum WeaponType : uint
	{
		None = 0,
		Pistol,
		Rifle,
		MAX = 3
	}

	public class WeaponManager : Entity
	{
		public uint startingWeapon;

		int currentWeapon;

		Entity ammoText;
		Entity rifle;
		Entity pistol;

		AudioSource weaponSwitchedSound;

		protected override void OnCreate()
		{
			ammoText = Scene.FindEntityByName("Ammo Text");
			rifle = Scene.FindEntityByName("Rifle");
			pistol = Scene.FindEntityByName("Pistol");
			weaponSwitchedSound = GetComponent<AudioSource>();

			currentWeapon = (int)startingWeapon;
			SetWeaponActive((WeaponType)currentWeapon);
		}

		protected override void OnUpdate(float delta)
		{
			SwitchWeaponOnScroll();
			SwitchWeaponOnKeyPress();
			SwitchWeaponGamepad();
		}

		public WeaponType GetCurrentWeapon()
		{
			return (WeaponType)currentWeapon;
		}

		void SwitchWeaponOnKeyPress()
		{
			if (Input.IsKeyDown(KeyCode.D1))
			{
				SwitchWeapon(WeaponType.None);
				currentWeapon = 0;
			}
			if (Input.IsKeyDown(KeyCode.D2))
			{
				SwitchWeapon(WeaponType.Pistol);
				currentWeapon = 1;
			}
			if (Input.IsKeyDown(KeyCode.D3))
			{
				SwitchWeapon(WeaponType.Rifle);
				currentWeapon = 2;
			}
		}

		void SwitchWeaponOnScroll()
		{
			Vector2 scrollDelta = Input.GetMouseWheelMovement();

			if (scrollDelta.Y > 0)
			{
				if (currentWeapon + 1 < (int)WeaponType.MAX)
					SwitchWeapon((WeaponType)(++currentWeapon));
			}
			else if (scrollDelta.Y < 0)
			{
				if (currentWeapon - 1 > -1)
					SwitchWeapon((WeaponType)(--currentWeapon));
			}
		}

		void SwitchWeaponGamepad()
		{
			if (Input.IsGamepadButtonDown(GamepadButton.Up))
			{
				SwitchWeapon(WeaponType.None);
				currentWeapon = (int)WeaponType.None;
			}
			if (Input.IsGamepadButtonDown(GamepadButton.Left))
			{
				SwitchWeapon(WeaponType.Pistol);
				currentWeapon = (int)WeaponType.Pistol;
			}
			if (Input.IsGamepadButtonDown(GamepadButton.Right))
			{
				SwitchWeapon(WeaponType.Rifle);
				currentWeapon = (int)WeaponType.Rifle;
			}
		}

		void SwitchWeapon(WeaponType weaponType)
		{
			SetWeaponActive(weaponType);
			weaponSwitchedSound.Play();
		}

		void HideWeapons()
		{
			pistol.SetActive(false);
			rifle.SetActive(false);
		}

		void SetWeaponActive(WeaponType weaponType)
		{
			switch (weaponType)
			{
				case WeaponType.None:
				{
					HideWeapons();
					ammoText.SetActive(false);
					break;
				}
				case WeaponType.Pistol:
				{
					HideWeapons();
					ammoText.SetActive(true);
					pistol.SetActive(true);
					break;
				}
				case WeaponType.Rifle:
				{
					HideWeapons();
					ammoText.SetActive(true);
					rifle.SetActive(true);
					break;
				}
			}
		}
	}

}

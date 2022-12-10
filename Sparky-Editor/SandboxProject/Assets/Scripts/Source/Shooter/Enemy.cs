using Sparky;

namespace Sandbox.Shooter.AI {

	using Shooter.Weapons;

	public class Enemy : Entity
	{
		public float moveSpeed;
		public float minDistanceToPlayer;

		Entity player;
		WeaponManager weaponManager;

		protected override void OnCreate()
		{
			player = FindEntityByName("Player");
			weaponManager = FindEntityByName("Weapon Manager").As<WeaponManager>();
		}

		protected override void OnUpdate(float delta)
		{
			if (weaponManager.GetCurrentWeapon() == WeaponType.None)
				return;

			FacePlayer();
			FollowPlayer();
		}

		private void FollowPlayer()
		{
			float distanceToPlayer = Vector3.Distance(transform.Translation, player.transform.Translation);

			if (distanceToPlayer > minDistanceToPlayer)
			{
				Vector3 displacement = transform.Forward * moveSpeed * Time.DeltaTime;
				transform.Translate(displacement.X, 0, displacement.Z);
			}
		}

		void FacePlayer()
		{
			transform.LookAt(player.transform.Translation);
		}
	}

}

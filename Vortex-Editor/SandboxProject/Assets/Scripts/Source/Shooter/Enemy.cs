using Vortex;

namespace Sandbox.Shooter.AI {

	using Shooter.Weapons;

	public class Enemy : Entity
	{
		public float moveSpeed;
		public float minDistanceToPlayer;
		public float followRadius;
		public float health;

		Entity player;
		WeaponManager weaponManager;
		bool isAggravated;
		bool wasHit;

		protected override void OnCreate()
		{
			player = FindEntityByName("Player");
			isAggravated = false;
		}

		protected override void OnUpdate(float delta)
		{
			weaponManager = FindEntityByName("Weapon Manager").As<WeaponManager>();

			if (GetDistanceToPlayer() < followRadius || wasHit)
			{
				isAggravated = true;
			}
			else if (GetDistanceToPlayer() > followRadius && !wasHit)
			{
				isAggravated = false;
			}

			if (weaponManager.GetCurrentWeapon() == WeaponType.None || !isAggravated)
			{
				wasHit = false;
				return;
			}

			FacePlayer();
			MoveTowardsPlayer();
		}

		public void OnEnemyHit(float damage)
		{
			health -= damage;

			if (health <= 0f)
			{
				Destroy(this);
			}

			wasHit = true;
		}

		float GetDistanceToPlayer() => Vector3.Distance(transform.Translation, player.transform.Translation);

		void MoveTowardsPlayer()
		{
			if (GetDistanceToPlayer() > minDistanceToPlayer)
			{
				Vector3 displacement = transform.Forward * moveSpeed * Time.DeltaTime;
				transform.Translate(displacement.X, 0, displacement.Z);
			}
		}

		void FacePlayer() => transform.LookAt(player.transform.Translation);
	}

}

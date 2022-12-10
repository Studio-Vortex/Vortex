using Sparky;

namespace Sandbox.Shooter.Collections {

	public class BulletPool
	{
		public bool IsFilled;
		Entity[] bullets;
		uint currentIndex;
		uint size;
		uint lastFreedIndex;

		const uint MAX_BULLETS = 25;

		public BulletPool()
		{
			IsFilled = false;
			bullets = new Entity[MAX_BULLETS];
			currentIndex = 0;
			size = 0;
			lastFreedIndex = 0;
		}

		public void AddBullet(Entity bullet)
		{
			bullets[currentIndex++] = bullet;
			size = currentIndex;

			if (size == MAX_BULLETS)
			{
				IsFilled = true;
				Debug.Info("BULLET POOL FILLED");
			}
		}

		public Entity Next()
		{
			if (lastFreedIndex + 1 > MAX_BULLETS - 1)
			{
				lastFreedIndex = 0;
			}

			if (size == 0 || bullets[lastFreedIndex] == null)
				return null;

			Entity next = bullets[lastFreedIndex++];
			return next;
		}

		public uint Size()
		{
			return size;
		}
	}

}

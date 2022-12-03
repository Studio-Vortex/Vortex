using Sparky;

namespace Sandbox {

	public class Ammo : Entity
	{
		public uint amount = 10;

		protected override void OnUpdate(float deltaTime)
		{
			if (Physics.Raycast(transform.Translation + transform.Up, transform.Up, 5f, out RaycastHit hitInfo))
			{
				Gun gun = FindEntityByName("Gun").As<Gun>();

				if (hitInfo.Entity.Tag == "Player")
				{
					gun.ammo += amount;
					GetComponent<AudioSource>().Play();
					Destroy(false);
				}
			}
		}
	}

}

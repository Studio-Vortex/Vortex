﻿using Vortex;

namespace Sandbox.Shooter.Weapons {

	public class Ammo : Entity
	{
		public uint amount = 10;

		protected override void OnUpdate(float deltaTime)
		{
			if (Physics.Raycast(transform.Translation + transform.Up, transform.Up, 5f, out RaycastHit hitInfo))
			{
				Gun gun = Scene.FindEntityByName("Pistol").As<Gun>();

				if (hitInfo.Entity.HasComponent<RigidBody>())
				{
					gun.AddAmmo(amount);
					GetComponent<AudioSource>().Play();
					Destroy(this);
				}
			}
		}
	}

}

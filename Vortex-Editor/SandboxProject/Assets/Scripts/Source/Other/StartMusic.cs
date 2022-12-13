using System;
using Vortex;

namespace Sandbox {

	public class StartMusic : Entity
	{
		protected override void OnCreate()
		{
			GetComponent<AudioSource>().Play();
		}
	}

}

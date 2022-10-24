using System;
using Sparky;

namespace Sandbox {

	public class StartMusic : Entity
	{
		public override void OnCreate()
		{
			GetComponent<AudioSource>().Play();
		}
	}

}

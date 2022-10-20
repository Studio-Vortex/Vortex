using System;
using Sparky;

namespace Sandbox {

	public class AA_OpeningScene : Entity
	{
		public override void OnCreate()
		{
			GetComponent<AudioSource>().Play();
		}
	}

}

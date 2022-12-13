using System;
using Vortex;

namespace Sandbox {

	public class A1_OpeningScene : Entity
	{
		protected override void OnCreate()
		{
			GetComponent<AudioSource>().Play();
		}
	}

}

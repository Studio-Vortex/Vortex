﻿using Vortex;

namespace Sandbox {

	public class AnimatorTest : Entity
	{
		void OnCreate()
		{
			if (TryGetComponent(out Animator animator))
			{
				animator.Play();
			}

			Debug.Log(animator.IsPlaying);
		}
	}

}

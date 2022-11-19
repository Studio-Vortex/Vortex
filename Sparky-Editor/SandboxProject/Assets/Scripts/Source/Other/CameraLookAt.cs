using System;
using Sparky;

namespace Sandbox {

	public class CameraLookAt : Entity
	{
		Camera m_Camera;
		AudioSource m_Sound;

		protected override void OnCreate()
		{
			m_Camera = GetComponent<Camera>();
			m_Sound = GetComponent<AudioSource>();
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsGamepadButtonDown(Gamepad.LeftBumper))
				m_Sound.Play();
		}
	}

}

using System;
using Sparky;

namespace Sandbox {

	public class CameraLookAt : Entity
	{
		Camera m_Camera;
		AudioSource m_Sound;

		public override void OnCreate()
		{
			m_Camera = GetComponent<Camera>();
			m_Sound = GetComponent<AudioSource>();
		}

		public override void OnUpdate(float delta)
		{
			if (Input.IsGamepadButtonDown(Gamepad.LeftBumper))
				m_Sound.Play();
		}
	}

}

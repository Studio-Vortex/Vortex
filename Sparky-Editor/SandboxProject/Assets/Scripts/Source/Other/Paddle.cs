﻿using System;
using Sparky;

namespace Sandbox {

	public class Paddle : Entity
	{
		public float Speed;
		private Vector3 m_Velocity;

		protected override void OnCreate()
		{
			Debug.Log("Welcome to Sparky!");
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsKeyDown(KeyCode.W) || Input.GetGamepadAxis(Gamepad.AxisLeftY) > 0.25f)
				m_Velocity.Y = 1.0f * -Input.GetGamepadAxis(Gamepad.AxisLeftY);
			if (Input.IsKeyDown(KeyCode.S) || Input.GetGamepadAxis(Gamepad.AxisLeftY) < -0.25f)
				m_Velocity.Y = -1.0f * Input.GetGamepadAxis(Gamepad.AxisLeftY);

			m_Velocity *= Speed * delta;
			transform.Translation += m_Velocity;

			if (transform.Translation.Y > 3.25f)
				transform.Translation = new Vector3(transform.Translation.X, 3.25f, transform.Translation.Z);
			if (transform.Translation.Y < -3.25f)
				transform.Translation = new Vector3(transform.Translation.X, -3.25f, transform.Translation.Z);
		}
	}

}
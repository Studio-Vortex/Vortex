using System;
using System.Collections.Generic;
using Sparky;

namespace Sandbox {

	public class CubeSpawner : Entity
	{
		public float WaitTime;
		public float MoveSpeed;
		public float RotationSpeed;
		public Vector3 MaxPosition;
		private float m_TimeToWait;
		private string[] m_Textures;
		private List<Entity> m_SpawnedCubes;
		private bool m_Flip = false;

		protected override void OnCreate()
		{
			m_TimeToWait = WaitTime;
			m_SpawnedCubes = new List<Entity>();
			m_Textures = new string[3];
			m_Textures[0] = "Crate.png";
			m_Textures[1] = "Crate_1.png";
			m_Textures[2] = "Crate_2.png";
		}

		protected override void OnUpdate(float delta)
		{
			if (Input.IsGamepadButtonDown(Gamepad.Up))
				SceneManager.LoadScene("Physics2D");
			if (Input.IsGamepadButtonDown(Gamepad.Down))
				SceneManager.LoadScene("Pong");
			if (Input.IsGamepadButtonDown(Gamepad.Left))
				SceneManager.LoadScene("ProceduralWorld");
			if (Input.IsGamepadButtonDown(Gamepad.Right))
				SceneManager.LoadScene("TestScene");

			WaitTime -= delta;

			if (WaitTime <= 0)
			{
				SpawnCube();
				WaitTime = m_TimeToWait;
			}

			foreach (Entity cube in m_SpawnedCubes)
			{
				float speed = MoveSpeed * delta;

				cube.transform.Translate(
					(float)Math.Sin(cube.transform.Rotation.X) * speed,
					(float)Math.Cos(cube.transform.Rotation.Y) * speed,
					(float)Math.Sin(cube.transform.Rotation.Z) * speed
				);

				cube.transform.Rotate(new Vector3(RotationSpeed * delta));
			}
		}

		void SpawnCube()
		{
			Entity cube = new Entity("Cube");

			float translationX = RandomDevice.RangedFloat(0.0f, MaxPosition.X);
			float translationY = RandomDevice.RangedFloat(1.0f, MaxPosition.Y);
			float translationZ = RandomDevice.RangedFloat(0.0f, MaxPosition.Z);

			if (m_Flip)
			{
				translationX *= -1;
				m_Flip = false;
			}
			else
			{
				translationZ *= -1;
				m_Flip = true;
			}

			cube.transform.Translate(translationX, translationY, translationZ);

			m_SpawnedCubes.Add(cube);
		}
	}

}

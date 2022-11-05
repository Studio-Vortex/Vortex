using System;
using System.Collections.Generic;
using Sparky;

namespace Sandbox {

	public class HowMany : Entity
	{
		public float Width;
		public float Height;
		public float Depth;
		List<Entity> m_Cubes;

		public override void OnCreate()
		{
			m_Cubes = new List<Entity>();

			for (float z = 0; z < Depth; z++)
			{
				for (float y = 0; y < Height; y++)
				{
					for (float x = 0; x < Width; x++)
					{
						/*Entity entity = new Entity($"{x}, {y}, {z}");
						entity.transform.Translate(x - Width / 2.0f, y, z);
						entity.AddComponent<MeshRenderer>().Color = new Vector4(
							RandomDevice.RangedFloat(0.0f, 1.0f),
							RandomDevice.RangedFloat(0.0f, 1.0f),
							RandomDevice.RangedFloat(0.0f, 1.0f),
							1.0f
						);*/

						//m_Cubes.Add(entity);
					}
				}
			}
		}

		public override void OnUpdate(float delta)
		{
			foreach (Entity cube in m_Cubes)
			{
				cube.transform.Translate(
					(float)Math.Sin(cube.transform.Rotation.X) * delta * 2.0f,
					(float)Math.Cos(cube.transform.Rotation.Y) * delta * 2.0f,
					(float)Math.Sin(cube.transform.Rotation.Z) * delta * 2.0f
				);

				cube.transform.Rotate(new Vector3(delta * 20.0f));
			}
		}
	}

}

using System.Collections.Generic;
using Vortex;

namespace Sandbox.ObstacleCourse {

	public class RandomColors : Entity
	{
		List<Entity> children;

		void OnCreate()
		{
			uint i = 0;

			children = new List<Entity>();

			while (true)
			{
				Entity child = GetChild(i++);

				if (child == null)
					break;

				if (child.TryGetComponent(out MeshRenderer meshRenderer))
				{
					Material material = meshRenderer.GetSubmesh(0).Material;
					float[] colors = new float[3] { Random.Float(0, 1), Random.Float(0, 1), Random.Float(0, 1) };
					material.Albedo = new Vector3(colors[0], colors[1], colors[2]);

					children.Add(child);
				}
			}
		}

		void OnDestroy()
		{
			foreach (Entity child in children)
			{
				Submesh submesh = child.GetComponent<MeshRenderer>().GetSubmesh(0);
				Material material = submesh.Material;
				material.Albedo = Color.White;
			}
		}
	}

}

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
					Material material = meshRenderer.GetMaterial();
					float[] colors = new float[3] { RandomDevice.RangedFloat(0, 1), RandomDevice.RangedFloat(0, 1), RandomDevice.RangedFloat(0, 1) };
					material.Albedo = new Vector3(colors[0], colors[1], colors[2]);

					children.Add(child);
				}
			}
		}

		void OnDestroy()
		{
			foreach (Entity child in children)
			{
				child.GetComponent<MeshRenderer>().GetMaterial().Albedo = Color.White.XYZ;
			}
		}
	}

}

using System.Collections.Generic;
using Vortex;

namespace Sandbox.Shooter {

	public class RandomColors : Entity
	{
		List<Entity> children;

		protected override void OnCreate()
		{
			uint i = 0;

			children = new List<Entity>();

			while (true)
			{
				Entity child = GetChild(i++);

				if (child == null)
					break;

				MeshRenderer meshRenderer = child.GetComponent<MeshRenderer>();
				Material material = meshRenderer.GetSubmesh(0).GetMaterial();
				float[] colors = new float[3] { RandomDevice.RangedFloat(0, 1), RandomDevice.RangedFloat(0, 1), RandomDevice.RangedFloat(0, 1) };
				material.Albedo = new Vector3(colors[0], colors[1], colors[2]);
				material.Metallic = 0;
				material.Roughness = 0;

				children.Add(child);
			}
		}
		
		protected override void OnUpdate(float delta)
		{

		}

		protected override void OnDestroy()
		{
			foreach (Entity entity in children)
			{
				if (entity.TryGetComponent(out MeshRenderer meshRenderer))
				{
					Material material = meshRenderer.GetSubmesh(0).GetMaterial();
					material.Albedo = Color.White;
					material.Metallic = 0.5f;
					material.Roughness = 0.5f;
				}
			}
		}
	}

}

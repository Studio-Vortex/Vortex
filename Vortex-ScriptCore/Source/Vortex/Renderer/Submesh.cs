namespace Vortex {

	public class Submesh
	{
		internal readonly uint SubmeshIndex;
		public Entity Entity;

		internal Submesh(uint index, Entity entity)
		{
			SubmeshIndex = index;
			Entity = entity;
		}

		public Material Material
		{
			get
			{
				InternalCalls.Material_GetAlbedo(Entity.ID, SubmeshIndex, out Vector3 albedo);
				InternalCalls.Material_GetUV(Entity.ID, SubmeshIndex, out Vector2 uv);

				return new Material(SubmeshIndex, Entity)
				{
					Albedo = albedo,
					Metallic = InternalCalls.Material_GetRoughness(Entity.ID, SubmeshIndex),
					Roughness = InternalCalls.Material_GetRoughness(Entity.ID, SubmeshIndex),
					Emission = InternalCalls.Material_GetEmission(Entity.ID, SubmeshIndex),
					UV = uv,
					Opacity = InternalCalls.Material_GetOpacity(Entity.ID, SubmeshIndex)
				};
			}
		}
	}

}

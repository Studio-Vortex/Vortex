namespace Vortex {

	public class Submesh
	{
		internal readonly uint Index;
		public Entity Entity;

		internal Submesh(uint index, Entity entity)
		{
			Index = index;
			Entity = entity;
		}

		public Material Material
		{
			get
			{
				InternalCalls.Material_GetAlbedo(Entity.ID, Index, out Vector3 albedo);
				InternalCalls.Material_GetUV(Entity.ID, Index, out Vector2 uv);

				return new Material(Index, Entity)
				{
					Albedo = albedo,
					Metallic = InternalCalls.Material_GetRoughness(Entity.ID, Index),
					Roughness = InternalCalls.Material_GetRoughness(Entity.ID, Index),
					Emission = InternalCalls.Material_GetEmission(Entity.ID, Index),
					UV = uv,
					Opacity = InternalCalls.Material_GetOpacity(Entity.ID, Index)
				};
			}
		}
	}

}

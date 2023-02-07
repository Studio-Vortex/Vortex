namespace Vortex {

	public struct Material
	{
		internal readonly uint SubmeshIndex;
		public Entity Entity;

		internal Material(uint submeshIndex, Entity entity)
		{
			SubmeshIndex = submeshIndex;
			Entity = entity;
		}

		public Vector3 Albedo
		{
			get
			{
				InternalCalls.Material_GetAlbedo(Entity.ID, SubmeshIndex, out Vector3 result);
				return result;
			}

			set => InternalCalls.Material_SetAlbedo(Entity.ID, SubmeshIndex, ref value);
		}

		public float Metallic
		{
			get => InternalCalls.Material_GetMetallic(Entity.ID, SubmeshIndex);
			set => InternalCalls.Material_SetMetallic(Entity.ID, SubmeshIndex, value);
		}

		public float Roughness
		{
			get => InternalCalls.Material_GetRoughness(Entity.ID, SubmeshIndex);
			set => InternalCalls.Material_SetRoughness(Entity.ID, SubmeshIndex, value);
		}

		public float Emission
		{
			get => InternalCalls.Material_GetEmission(Entity.ID, SubmeshIndex);
			set => InternalCalls.Material_SetEmission(Entity.ID, SubmeshIndex, value);
		}

		public Vector2 UV
		{
			get
			{
				InternalCalls.Material_GetUV(Entity.ID, SubmeshIndex, out Vector2 result);
				return result;
			}

			set => InternalCalls.Material_SetUV(Entity.ID, SubmeshIndex, ref value);
		}

		public float Opacity
		{
			get => InternalCalls.Material_GetOpacity(Entity.ID, SubmeshIndex);
			set => InternalCalls.Material_SetOpacity(Entity.ID, SubmeshIndex, value);
		}
	}

}

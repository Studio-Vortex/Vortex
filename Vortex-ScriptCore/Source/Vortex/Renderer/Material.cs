namespace Vortex {

	public class Material
	{
		public Entity Entity;

		public Vector3 Albedo
		{
			get
			{
				InternalCalls.Material_GetAlbedo(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.Material_SetAlbedo(Entity.ID, ref value);
		}

		public float Metallic
		{
			get => InternalCalls.Material_GetMetallic(Entity.ID);
			set => InternalCalls.Material_SetMetallic(Entity.ID, value);
		}

		public float Roughness
		{
			get => InternalCalls.Material_GetRoughness(Entity.ID);
			set => InternalCalls.Material_SetRoughness(Entity.ID, value);
		}

		public Vector3 Emission
		{
			get
			{
				InternalCalls.Material_GetEmission(Entity.ID, out Vector3 result);
				return result;
			}

			set => InternalCalls.Material_SetEmission(Entity.ID, ref value);
		}

		public float Opacity
		{
			get => InternalCalls.Material_GetOpacity(Entity.ID);
			set => InternalCalls.Material_SetOpacity(Entity.ID, value);
		}
	}

}

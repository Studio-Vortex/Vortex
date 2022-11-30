namespace Sparky {

	public class Material
	{
		public Entity Entity;

		public Vector3 Albedo
		{
			get
			{
				InternalCalls.MeshRendererComponent_GetMaterialAlbedo(Entity.ID, out Vector3 result);
				return result;
			}

			set
			{
				InternalCalls.MeshRendererComponent_SetMaterialAlbedo(Entity.ID, ref value);
			}
		}
	}

}

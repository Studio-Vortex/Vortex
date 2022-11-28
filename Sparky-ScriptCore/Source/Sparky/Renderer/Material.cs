namespace Sparky {

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

			set
			{
				InternalCalls.Material_SetAlbedo(Entity.ID, ref value);
			}
		}
	}

}

namespace Sparky {

	public class Material
	{
		public Vector3 Albedo
		{
			get
			{
				InternalCalls.Material_GetAlbedo(m_Entity.ID, out Vector3 result);
				return result;
			}

			set
			{
				InternalCalls.Material_SetAlbedo(m_Entity.ID, ref value);
			}
		}

		private Entity m_Entity;

		internal Material(ulong entityID)
		{
			m_Entity = new Entity(entityID);
		}
	}

}

namespace Vortex {

	public class Submesh
	{
		internal AssetHandle m_MaterialHandle;
		internal Material m_Material;

		internal Submesh() { m_MaterialHandle = AssetHandle.Invalid; }
		internal Submesh(AssetHandle materialHandle)
		{
			m_MaterialHandle = materialHandle;

			InternalCalls.Material_GetAlbedo(ref m_MaterialHandle, out Vector3 albedo);
			InternalCalls.Material_GetUV(ref m_MaterialHandle, out Vector2 uv);

			m_Material = new Material(m_MaterialHandle)
			{
				Albedo = albedo,
				Metallic = InternalCalls.Material_GetRoughness(ref m_MaterialHandle),
				Roughness = InternalCalls.Material_GetRoughness(ref m_MaterialHandle),
				Emission = InternalCalls.Material_GetEmission(ref m_MaterialHandle),
				UV = uv,
				Opacity = InternalCalls.Material_GetOpacity(ref m_MaterialHandle)
			};
		}

		public Material Material => m_Material;
	}

}

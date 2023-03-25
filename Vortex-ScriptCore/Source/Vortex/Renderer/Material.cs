namespace Vortex {

	public enum MaterialFlag
	{
		None = 0,
		NoDepthTest = 1 << 1,
	}

	public class Material
	{
		internal AssetHandle m_Handle;
		public AssetHandle Handle => m_Handle;

		internal Material() { m_Handle = AssetHandle.Invalid; }
		internal Material(AssetHandle assetHandle)
		{
			m_Handle = assetHandle;
		}

		public Vector3 Albedo
		{
			get
			{
				InternalCalls.Material_GetAlbedo(ref m_Handle, out Vector3 result);
				return result;
			}

			set => InternalCalls.Material_SetAlbedo(ref m_Handle, ref value);
		}

		public Texture2D AlbedoMap
		{
			get
			{
				return InternalCalls.Material_GetAlbedoMap(ref m_Handle, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set
			{
				AssetHandle textureHandle = value.Handle;
				InternalCalls.Material_SetAlbedoMap(ref m_Handle, ref textureHandle);
			}
		}

		public Texture2D NormalMap
		{
			get
			{
				return InternalCalls.Material_GetNormalMap(ref m_Handle, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set
			{
				AssetHandle textureHandle = value.Handle;
				InternalCalls.Material_SetNormalMap(ref m_Handle, ref textureHandle);
			}
		}

		public float Metallic
		{
			get => InternalCalls.Material_GetMetallic(ref m_Handle);
			set => InternalCalls.Material_SetMetallic(ref m_Handle, value);
		}

		public Texture2D MetallicMap
		{
			get
			{
				return InternalCalls.Material_GetMetallicMap(ref m_Handle, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set
			{
				AssetHandle textureHandle = value.Handle;
				InternalCalls.Material_SetMetallicMap(ref m_Handle, ref textureHandle);
			}
		}

		public float Roughness
		{
			get => InternalCalls.Material_GetRoughness(ref m_Handle);
			set => InternalCalls.Material_SetRoughness(ref m_Handle, value);
		}

		public Texture2D RoughnessMap
		{
			get
			{
				return InternalCalls.Material_GetRoughnessMap(ref m_Handle, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set
			{
				AssetHandle textureHandle = value.Handle;
				InternalCalls.Material_SetRoughnessMap(ref m_Handle, ref textureHandle);
			}
		}

		public float Emission
		{
			get => InternalCalls.Material_GetEmission(ref m_Handle);
			set => InternalCalls.Material_SetEmission(ref m_Handle, value);
		}

		public Texture2D EmissionMap
		{
			get
			{
				return InternalCalls.Material_GetEmissionMap(ref m_Handle, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set
			{
				AssetHandle textureHandle = value.Handle;
				InternalCalls.Material_SetEmissionMap(ref m_Handle, ref textureHandle);
			}
		}

		public Texture2D AmbientOcclusionMap
		{
			get
			{
				return InternalCalls.Material_GetAmbientOcclusionMap(ref m_Handle, out AssetHandle textureHandle)
					? new Texture2D(textureHandle) : null;
			}

			set
			{
				AssetHandle textureHandle = value.Handle;
				InternalCalls.Material_SetAmbientOcclusionMap(ref m_Handle, ref textureHandle);
			}
		}

		public Vector2 UV
		{
			get
			{
				InternalCalls.Material_GetUV(ref m_Handle, out Vector2 result);
				return result;
			}

			set => InternalCalls.Material_SetUV(ref m_Handle, ref value);
		}

		public float Opacity
		{
			get => InternalCalls.Material_GetOpacity(ref m_Handle);
			set => InternalCalls.Material_SetOpacity(ref m_Handle, value);
		}

		public bool IsFlagSet(MaterialFlag flag) => InternalCalls.Material_IsFlagSet(ref m_Handle, flag);
		public void SetFlag(MaterialFlag flag, bool value) => InternalCalls.Material_SetFlag(ref m_Handle, flag, value);
	}

}

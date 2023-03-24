using System;

namespace Vortex {

	public class Texture2D
	{
		internal AssetHandle m_Handle;
		public AssetHandle Handle => m_Handle;

		internal Texture2D() { m_Handle = AssetHandle.Invalid; }
		internal Texture2D(AssetHandle assetHandle)
		{
			m_Handle = assetHandle;
		}

		public Texture2D(string filepath)
		{
			InternalCalls.Texture2D_LoadFromPath(filepath, out m_Handle);
		}

		public Texture2D(uint width, uint height)
		{
			InternalCalls.Texture2D_Constructor(width, height, out m_Handle);
		}

		public float Width => InternalCalls.Texture2D_GetWidth(ref m_Handle);
		public float Height => InternalCalls.Texture2D_GetHeight(ref m_Handle);

		public void SetPixel(uint x, uint y, Color4 color)
		{
			InternalCalls.Texture2D_SetPixel(ref m_Handle, x, y, ref color);
		}
	}

}

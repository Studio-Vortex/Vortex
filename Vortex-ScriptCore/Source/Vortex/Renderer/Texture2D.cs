using System;

namespace Vortex {

	public class Texture2D: Asset
	{
		internal IntPtr m_UnmanagedInstance;

		internal Texture2D(IntPtr unmanagedInstance)
		{
			m_UnmanagedInstance = unmanagedInstance;
		}

		public Texture2D(string filepath)
		{
			m_UnmanagedInstance = InternalCalls.Texture2D_LoadFromPath(filepath);
		}

		public Texture2D(uint width, uint height)
		{
			m_UnmanagedInstance = InternalCalls.Texture2D_Constructor(width, height);
		}

		public float Width => InternalCalls.Texture2D_GetWidth(m_UnmanagedInstance);
		public float Height => InternalCalls.Texture2D_GetHeight(m_UnmanagedInstance);

		public void SetPixel(uint x, uint y, Color4 color)
		{
			InternalCalls.Texture2D_SetPixel(m_UnmanagedInstance, x, y, ref color);
		}
	}

}

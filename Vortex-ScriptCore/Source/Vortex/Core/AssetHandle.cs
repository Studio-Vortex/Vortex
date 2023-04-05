using System.Runtime.InteropServices;

namespace Vortex {

	[StructLayout(LayoutKind.Sequential)]
	public struct AssetHandle
	{
		public static readonly AssetHandle Invalid = new AssetHandle(0);

		private ulong m_Handle;

		public AssetHandle(ulong handle)
		{
			m_Handle = handle;
		}

		public bool IsValid() => InternalCalls.AssetHandle_IsValid(ref this);

		public static implicit operator bool(AssetHandle assetHandle)
		{
			return assetHandle.IsValid();
		}

		public override string ToString() => m_Handle.ToString();
		public override int GetHashCode() => m_Handle.GetHashCode();
	}

}

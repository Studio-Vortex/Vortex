namespace Vortex {

    public class PhysicsMaterial
    {
        internal AssetHandle m_Handle;
		public AssetHandle Handle => m_Handle;

		internal PhysicsMaterial() { m_Handle = AssetHandle.Invalid; }
		internal PhysicsMaterial(AssetHandle handle)
		{
			m_Handle = handle;
		}

		public float StaticFriction
		{
			get => InternalCalls.PhysicsMaterial_GetStaticFriction(ref m_Handle);
			set => InternalCalls.PhysicsMaterial_SetStaticFriction(ref m_Handle, value);
		}

		public float DynamicFriction
		{
			get => InternalCalls.PhysicsMaterial_GetDynamicFriction(ref m_Handle);
			set => InternalCalls.PhysicsMaterial_SetDynamicFriction(ref m_Handle, value);
		}

		public float Bounciness
		{
			get => InternalCalls.PhysicsMaterial_GetBounciness(ref m_Handle);
			set => InternalCalls.PhysicsMaterial_SetBounciness(ref m_Handle, value);
		}

		public CombineMode FrictionCombine
		{
			get
			{
				CombineMode result = InternalCalls.PhysicsMaterial_GetFrictionCombineMode(ref m_Handle);
				return result;
			}

			set => InternalCalls.PhysicsMaterial_SetFrictionCombineMode(ref m_Handle, value);
		}

		public CombineMode BouncinessCombine
		{
			get
			{
				CombineMode result = InternalCalls.PhysicsMaterial_GetBouncinessCombineMode(ref m_Handle);
				return result;
			}

			set => InternalCalls.PhysicsMaterial_SetBouncinessCombineMode(ref m_Handle, value);
		}
	}

}

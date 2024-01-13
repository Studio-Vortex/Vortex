namespace Vortex {

	public struct DropShadowInfo
	{
		public Actor Actor;

		internal DropShadowInfo(Actor actor)
		{
			Actor = actor;
		}

		public Color4 Color
		{
			get
			{
				InternalCalls.DropShadowInfo_GetColor(Actor.ID, out Color4 result);
				return result;
			}

			set => InternalCalls.DropShadowInfo_SetColor(Actor.ID, ref value);
		}

		public Vector2 ShadowDistance
		{
			get
			{
				InternalCalls.DropShadowInfo_GetShadowDistance(Actor.ID, out Vector2 result);
				return result;
			}

			set => InternalCalls.DropShadowInfo_SetShadowDistance(Actor.ID, ref value);
		}

		public float ShadowScale
		{
			get => InternalCalls.DropShadowInfo_GetShadowScale(Actor.ID);
			set => InternalCalls.DropShadowInfo_SetShadowScale(Actor.ID, value);
		}

		public bool Enabled
		{
			get => InternalCalls.DropShadowInfo_GetEnabled(Actor.ID);
			set => InternalCalls.DropShadowInfo_SetEnabled(Actor.ID, value);
		}
	}

}

namespace Vortex {

	public struct RaycastHit2D
	{
		public Vector2 Point;
		public Vector2 Normal;
		public string Tag;
		public bool Hit;
	}

	public static class Physics2D
	{
		public static Vector2 Gravity
		{
			get
			{
				InternalCalls.Physics2D_GetWorldGravity(out Vector2 result);
				return result;
			}
			set => InternalCalls.Physics2D_SetWorldGravity(ref value);
		}

		public static uint PositionIterations
		{
			get => InternalCalls.Physics2D_GetWorldPositionIterations();
			set => InternalCalls.Physics2D_SetWorldPositionIterations(value);
		}

		public static uint VelocityIterations
		{
			get => InternalCalls.Physics2D_GetWorldVelocityIterations();
			set => InternalCalls.Physics2D_SetWorldVelocityIterations(value);
		}

		public static Actor Raycast(Vector2 start, Vector2 end, out RaycastHit2D hit, bool drawDebugLine = false)
		{
			ulong entityID = InternalCalls.Physics2D_Raycast(ref start, ref end, out hit, drawDebugLine);

			if (entityID == 0)
				return null;

			return new Actor(entityID);
		}
	}

}

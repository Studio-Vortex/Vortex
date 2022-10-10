namespace Sparky {

	public struct RayCastHit2D
	{
		public Vector2 Point;
		public Vector2 Normal;
		public string Name;
		public bool Hit;
	}

	public static class Physics2D
	{
		public static Entity Raycast(Vector2 start, Vector2 end, out RayCastHit2D hit, bool drawDebugLine = false)
		{
			ulong entityID = InternalCalls.Physics2D_Raycast(ref start, ref end, out hit, drawDebugLine);

			if (entityID != 0)
				return new Entity(entityID);

			return null;
		}
	}

}

namespace Sparky {

	public struct RayCastHit2D
	{
		public Vector2 Point;
		public Vector2 Normal;
		public ulong __EntityID;
		public bool Hit;
	}

	public static class Physics2D
	{
		public static Entity Raycast(Vector2 start, Vector2 end, out RayCastHit2D hit, bool drawDebugLine = false)
		{
			InternalCalls.Physics2D_Raycast(ref start, ref end, out hit, drawDebugLine);

			if (hit.Hit)
				return new Entity(hit.__EntityID);

			return null;
		}
	}

}

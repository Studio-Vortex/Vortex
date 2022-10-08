namespace Sparky {

	public static class Physics2D
	{
		public static bool Raycast(Vector2 origin, Vector2 direction, bool drawDebugLine = false)
		{
			return InternalCalls.Physics2D_Raycast(ref origin, ref direction, drawDebugLine);
		}
	}

}

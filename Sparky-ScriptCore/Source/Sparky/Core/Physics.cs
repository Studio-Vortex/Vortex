namespace Sparky {

	public struct RayCastHit2D
	{
		public Vector2 Point;
		public Vector2 Normal;
		public string Tag;
		public bool Hit;
	}

	public static class Physics2D
	{
		public static Entity Raycast(Vector2 start, Vector2 end, out RayCastHit2D hit, bool drawDebugLine = false)
		{
			ulong entityID = InternalCalls.Physics2D_Raycast(ref start, ref end, out hit, drawDebugLine);

			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}
	}

	public struct RaycastHit
	{
		public ulong EntityID { get; private set; }
		public Vector3 Position { get; private set; }
		public Vector3 Normal { get; private set; }
		public float Distance { get; private set; }

		public Entity Entity => Scene.FindEntityByID(EntityID);
	}

	public static class Physics
	{
		public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit)
		{
			return InternalCalls.Physics_Raycast(ref origin, ref direction, maxDistance, out hit);
		}
	}

}

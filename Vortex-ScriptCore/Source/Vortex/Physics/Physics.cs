namespace Vortex {

	public struct RaycastHit
	{
		public ulong EntityID { get; private set; }
		public Vector3 Position { get; private set; }
		public Vector3 Normal { get; private set; }
		public float Distance { get; private set; }

		public Entity Entity
		{
			get => Scene.FindEntityByID(EntityID);
		}
	}

	public static class Physics
	{
		public static Vector3 Gravity
		{
			get
			{
				InternalCalls.Physics_GetSceneGravity(out Vector3 result);
				return result;
			}
			set => InternalCalls.Physics_SetSceneGravity(ref value);
		}

		public static uint PositionIterations
		{
			get => InternalCalls.Physics_GetScenePositionIterations();
			set => InternalCalls.Physics_SetScenePositionIterations(value);
		}

		public static uint VelocityIterations
		{
			get => InternalCalls.Physics_GetSceneVelocityIterations();
			set => InternalCalls.Physics_SetSceneVelocityIterations(value);
		}

		public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit)
		{
			return InternalCalls.Physics_Raycast(ref origin, ref direction, maxDistance, out hit);
		}
	}

	public struct Collision
	{
		public ulong EntityID { get; private set; }

		public Entity Entity
		{
			get => Scene.FindEntityByID(EntityID);
		}
	}

}

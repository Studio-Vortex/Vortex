namespace Vortex {

	public struct RayCastHit2D
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

namespace Vortex {

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

		public static bool Raycast(Ray ray, float maxDistance, out RaycastHit hit)
		{
			return Raycast(ray.Origin, ray.Direction, maxDistance, out hit);
		}

		public static bool Raycast(Vector3 origin, Vector3 direction, float maxDistance, out RaycastHit hit)
		{
			return InternalCalls.Physics_Raycast(ref origin, ref direction, maxDistance, out hit);
		}
	}

}

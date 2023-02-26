namespace Vortex {

	public static class Scene
	{
		public static Entity FindEntityByID(ulong entityID)
		{
			bool found = InternalCalls.Scene_FindEntityByID(entityID);

			if (!found)
				return null;

			return new Entity(entityID);
		}

		public static Entity FindEntityByName(string name)
		{
			ulong entityID = InternalCalls.Scene_FindEntityByName(name);

			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public static Entity CreateEntity(string name = "")
		{
			ulong entityID = InternalCalls.Scene_CreateEntity(name);
			return new Entity(entityID);
		}

		// Clones the entity and returns it
		public static Entity Instantiate(Entity entity)
		{
			ulong entityID = InternalCalls.Scene_Instantiate(entity.ID);

			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public static Entity Instantiate(Entity entity, Vector3 worldPosition)
		{
			ulong entityID = InternalCalls.Scene_InstantiateAtWorldPos(entity.ID, ref worldPosition);

			if (entityID == 0)
				return null;
			
			return new Entity(entityID);
		}

		public static Entity Instantiate(Entity entity, Entity parent, Vector3 worldPosition)
		{
			ulong entityID = InternalCalls.Scene_InstantiateAtWorldPositionWithParent(entity.ID, parent.ID, ref worldPosition);

			if (entityID == 0)
				return null;
			
			return new Entity(entityID);
		}

		public static Entity

		public static bool IsPaused()
		{
			return InternalCalls.Scene_IsPaused();
		}

		public static void Pause()
		{
			InternalCalls.Scene_Pause();
		}

		public static void Resume()
		{
			InternalCalls.Scene_Resume();
		}

		public static Entity GetHoveredEntity()
		{
			ulong entityID = InternalCalls.Scene_GetHoveredEntity();

			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}
	}

}

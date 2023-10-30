﻿namespace Vortex {

	public static class Scene
	{
		public static uint BuildIndex = InternalCalls.Scene_GetCurrentBuildIndex();

		public static Entity PrimaryCamera
		{
			get
			{
				ulong entityID = InternalCalls.Scene_GetPrimaryCamera();

				if (entityID == 0)
					return null;

				return new Entity(entityID);
			}
		}

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
			ulong entityID = InternalCalls.Scene_Instantiate(entity.ID);

			if (entityID == 0)
				return null;

			Entity e = new Entity(entityID);

			entity.transform.Translation = worldPosition;

			return e;
		}

		public static Entity Instantiate(Entity entity, Vector3 worldPosition, Quaternion orientation)
		{
			ulong entityID = InternalCalls.Scene_Instantiate(entity.ID);

			if (entityID == 0)
				return null;

			Entity e = new Entity(entityID);

			entity.transform.Translation = worldPosition;
			entity.transform.Rotation = orientation;

			return e;
		}

		public static Entity Instantiate(Entity entity, Entity parent)
		{
			ulong entityID = InternalCalls.Scene_InstantiateAsChild(entity.ID, parent.ID);

			if (entityID == 0)
				return null;
			
			return new Entity(entityID);
		}

		public static Entity Instantiate(Entity entity, Entity parent, Vector3 worldPosition)
		{
			ulong entityID = InternalCalls.Scene_InstantiateAsChild(entity.ID, parent.ID);

			if (entityID == 0)
				return null;

			Entity e = new Entity(entityID);

			entity.transform.Translation = worldPosition;

			return e;
		}

		public static Entity Instantiate(Entity entity, Entity parent, Vector3 worldPosition, Quaternion orientation)
		{
			ulong entityID = InternalCalls.Scene_InstantiateAsChild(entity.ID, parent.ID);

			if (entityID == 0)
				return null;

			Entity e = new Entity(entityID);

			entity.transform.Translation = worldPosition;
			entity.transform.Rotation = orientation;

			return e;
		}

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

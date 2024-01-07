namespace Vortex {

	public static class Scene
	{
		public static Actor FindActorByID(ulong actorID)
		{
			bool found = InternalCalls.Scene_FindActorByID(actorID);

			if (!found)
				return null;

			return new Actor(actorID);
		}

		public static Actor FindActorByName(string name)
		{
			ulong actorID = InternalCalls.Scene_FindActorByName(name);

			if (actorID == 0)
				return null;

			return new Actor(actorID);
		}

		public static Actor FindChildByName(Actor actor, string name)
		{
			ulong actorID = InternalCalls.Scene_FindChildByName(actor.ID, name);

			if (actorID == 0)
				return null;

			return new Actor(actorID);
		}

		public static Actor CreateActor(string name = "")
		{
			ulong actorID = InternalCalls.Scene_CreateActor(name);
			return new Actor(actorID);
		}

		// Clones the actor and returns it
		public static Actor Instantiate(Actor actor)
		{
			ulong actorID = InternalCalls.Scene_Instantiate(actor.ID);

			if (actorID == 0)
				return null;

			return new Actor(actorID);
		}

		public static Actor Instantiate(Actor actor, Vector3 worldPosition)
		{
			ulong actorID = InternalCalls.Scene_Instantiate(actor.ID);

			if (actorID == 0)
				return null;

			Actor clone = new Actor(actorID);

			clone.transform.Translation = worldPosition;

			return clone;
		}

		public static Actor Instantiate(Actor actor, Vector3 worldPosition, Quaternion orientation)
		{
			ulong actorID = InternalCalls.Scene_Instantiate(actor.ID);

			if (actorID == 0)
				return null;

			Actor clone = new Actor(actorID);

			clone.transform.Translation = worldPosition;
			clone.transform.Rotation = orientation;

			return clone;
		}

		public static Actor Instantiate(Actor actor, Actor parent)
		{
			ulong actorID = InternalCalls.Scene_InstantiateAsChild(actor.ID, parent.ID);

			if (actorID == 0)
				return null;
			
			return new Actor(actorID);
		}

		public static Actor Instantiate(Actor actor, Actor parent, Vector3 worldPosition)
		{
			ulong actorID = InternalCalls.Scene_InstantiateAsChild(actor.ID, parent.ID);

			if (actorID == 0)
				return null;

			Actor clone = new Actor(actorID);

			clone.transform.Translation = worldPosition;

			return clone;
		}

		public static Actor Instantiate(Actor actor, Actor parent, Vector3 worldPosition, Quaternion orientation)
		{
			ulong actorID = InternalCalls.Scene_InstantiateAsChild(actor.ID, parent.ID);

			if (actorID == 0)
				return null;

			Actor clone = new Actor(actorID);

			clone.transform.Translation = worldPosition;
			clone.transform.Rotation = orientation;

			return clone;
		}

		public static bool IsPaused() => InternalCalls.Scene_IsPaused();

		public static void Pause() => InternalCalls.Scene_Pause();

		public static void Resume() => InternalCalls.Scene_Resume();
	}

}

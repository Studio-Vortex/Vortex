using System;

namespace Vortex {

	public static class Scene
	{
		public static Actor FindActorByID(ulong actorID)
		{
			bool found = InternalCalls.Scene_FindActorByID(actorID);

			if (!found) {
				return null;
			}

			return new Actor(actorID);
		}

		public static Actor FindActorByTag(string tag)
		{
			ulong actorID = InternalCalls.Scene_FindActorByTag(tag);

			if (actorID == 0) {
				return null;
			}

			return new Actor(actorID);
		}

		// Finds the first actor with a specific user generated script type
		public static T FindActorByType<T>()
			where T : Actor, new()
		{
			Type derivedType = typeof(T);
			object instance = InternalCalls.Scene_FindActorByType(derivedType);
			return instance as T;
		}

		public static bool TryFindActorByType<T>(out T instance)
			where T : Actor, new()
		{
			T foundInstance = FindActorByType<T>();

			if (!foundInstance) {
				instance = null;
				return false;
			}

			instance = foundInstance;
			return true;
		}

		public static Actor FindChildByTag(Actor parent, string tag)
		{
			ulong actorID = InternalCalls.Scene_FindChildByTag(parent.ID, tag);

			if (actorID == 0) {
				return null;
			}

			return new Actor(actorID);
		}

		// Finds the first child with a specific user generated script type
		public static T FindChildByType<T>(Actor parent)
			where T : Actor, new()
		{
			Type derivedType = typeof(T);
			object instance = InternalCalls.Scene_FindChildByType(parent.ID, derivedType);
			return instance as T;
		}

		public static bool TryFindChildByType<T>(Actor parent, out T instance)
			where T : Actor, new()
		{
			T foundInstance = FindChildByType<T>(parent);

			if (!foundInstance)
			{
				instance = null;
				return false;
			}

			instance = foundInstance;
			return true;
		}

		public static Actor CreateActor(string tag = "")
		{
			ulong actorID = InternalCalls.Scene_CreateActor(tag);
			return new Actor(actorID);
		}

		// Clones the actor and returns it
		public static Actor Instantiate(Actor actor)
		{
			ulong actorID = InternalCalls.Scene_Instantiate(actor.ID);

			if (actorID == 0) {
				return null;
			}

			return new Actor(actorID);
		}

		public static Actor Instantiate(Actor actor, Vector3 worldPosition)
		{
			ulong actorID = InternalCalls.Scene_Instantiate(actor.ID);

			if (actorID == 0) {
				return null;
			}

			Actor clone = new Actor(actorID);

			clone.transform.Translation = worldPosition;

			return clone;
		}

		public static Actor Instantiate(Actor actor, Vector3 worldPosition, Quaternion orientation)
		{
			ulong actorID = InternalCalls.Scene_Instantiate(actor.ID);

			if (actorID == 0) {
				return null;
			}

			Actor clone = new Actor(actorID);

			clone.transform.Translation = worldPosition;
			clone.transform.Rotation = orientation;

			return clone;
		}

		public static Actor Instantiate(Actor actor, Actor parent)
		{
			ulong actorID = InternalCalls.Scene_InstantiateAsChild(actor.ID, parent.ID);

			if (actorID == 0) {
				return null;
			}

			return new Actor(actorID);
		}

		public static Actor Instantiate(Actor actor, Actor parent, Vector3 worldPosition)
		{
			ulong actorID = InternalCalls.Scene_InstantiateAsChild(actor.ID, parent.ID);

			if (actorID == 0) {
				return null;
			}

			Actor clone = new Actor(actorID);

			clone.transform.Translation = worldPosition;

			return clone;
		}

		public static Actor Instantiate(Actor actor, Actor parent, Vector3 worldPosition, Quaternion orientation)
		{
			ulong actorID = InternalCalls.Scene_InstantiateAsChild(actor.ID, parent.ID);

			if (actorID == 0) {
				return null;
			}

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

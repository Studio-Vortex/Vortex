﻿namespace Sparky {

	public static class Scene
	{
		public static Entity FindEntityByID(ulong entityID)
		{
			bool found = InternalCalls.Scene_FindEntityByID(entityID);
			if (found)
				return new Entity(entityID);

			return null;
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
	}

}

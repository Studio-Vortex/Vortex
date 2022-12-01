namespace Sparky {

	public static class Scene
	{
		public static Entity FindEntityByID(ulong entityID)
		{
			ulong result = InternalCalls.Scene_FindEntityByID(entityID);

			if (result != 0)
				return new Entity(result);

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

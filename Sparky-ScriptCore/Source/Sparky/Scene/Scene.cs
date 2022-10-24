namespace Sparky {

	public static class Scene
	{
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

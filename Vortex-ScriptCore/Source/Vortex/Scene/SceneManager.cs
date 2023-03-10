namespace Vortex {

	public static class SceneManager
	{
		public static void LoadScene(string sceneName) => InternalCalls.SceneManager_LoadScene(sceneName);
		public static void LoadScene(uint buildIndex) => InternalCalls.SceneManager_LoadSceneFromBuildIndex(buildIndex);

		public static string GetActiveScene() => InternalCalls.SceneManager_GetActiveScene();
	}

}

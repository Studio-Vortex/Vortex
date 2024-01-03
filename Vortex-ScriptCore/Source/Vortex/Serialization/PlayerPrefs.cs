namespace Vortex {
	
	public static class PlayerPrefs
	{
		public static bool HasKey(string key) => InternalCalls.PlayerPrefs_HasKey(key);

		public static bool Remove(string key) => InternalCalls.PlayerPrefs_RemoveKey(key);

		public static void WriteInt(string key, int value) => InternalCalls.PlayerPrefs_WriteInt(key, value);
		public static int ReadInt(string key) => InternalCalls.PlayerPrefs_ReadInt(key);
		public static int ReadInt(string key, int defaultValue) => InternalCalls.PlayerPrefs_ReadIntWithDefault(key, defaultValue);
	}

}

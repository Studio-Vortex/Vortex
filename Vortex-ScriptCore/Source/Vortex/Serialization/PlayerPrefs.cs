namespace Vortex {
	
	public static class PlayerPrefs
	{
		public static bool HasKey(string key) => InternalCalls.PlayerPrefs_HasKey(key);

		public static bool Remove(string key) => InternalCalls.PlayerPrefs_RemoveKey(key);

		public static void WriteInt(string key, int value) => InternalCalls.PlayerPrefs_WriteInt(key, value);
		public static int ReadInt(string key) => InternalCalls.PlayerPrefs_ReadInt(key);
		public static int ReadInt(string key, int defaultValue) => InternalCalls.PlayerPrefs_ReadIntWithDefault(key, defaultValue);

		public static void WriteString(string key, string value) => InternalCalls.PlayerPrefs_WriteString(key, value);
		public static string ReadString(string key) => InternalCalls.PlayerPrefs_ReadString(key);
		public static string ReadString(string key, string defaultValue) => InternalCalls.PlayerPrefs_ReadStringWithDefault(key, defaultValue);
	}

}

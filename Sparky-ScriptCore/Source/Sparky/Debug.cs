namespace Sparky {

	public static class Debug
	{
		public static void Log(string message)
		{
			InternalCalls.Debug_Log(message);
		}

		public static void Info(string message)
		{
			InternalCalls.Debug_Info(message);
		}

		public static void Warn(string message)
		{
			InternalCalls.Debug_Warn(message);
		}

		public static void Error(string message)
		{
			InternalCalls.Debug_Error(message);
		}

		public static void Critical(string message)
		{
			InternalCalls.Debug_Critical(message);
		}
	}

}

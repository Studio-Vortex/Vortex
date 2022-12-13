namespace Vortex {

	public static class Debug
	{
		public static void Log(string message) => InternalCalls.Debug_Log(message);
		public static void Log(object message)
		{
			Log(message.ToString());
		}
		
		public static void Info(string message) => InternalCalls.Debug_Info(message);
		public static void Info(object message)
		{
			Info(message.ToString());
		}

		public static void Warn(string message) => InternalCalls.Debug_Warn(message);
		public static void Warn(object message)
		{
			Warn(message.ToString());
		}

		public static void Error(string message) => InternalCalls.Debug_Error(message);
		public static void Error(object message)
		{
			Error(message.ToString());
		}

		public static void Critical(string message) => InternalCalls.Debug_Critical(message);
		public static void Critical(object message)
		{
			Critical(message.ToString());
		}
	}

}

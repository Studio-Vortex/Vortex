namespace Vortex {

	public static class Log
	{
		public static void Print(string message) => InternalCalls.Log_Print(message);
		public static void Print(object message) => Print(message.ToString());
		
		public static void Info(string message) => InternalCalls.Log_Info(message);
		public static void Info(object message) => Info(message.ToString());

		public static void Warn(string message) => InternalCalls.Log_Warn(message);
		public static void Warn(object message) => Warn(message.ToString());

		public static void Error(string message) => InternalCalls.Log_Error(message);
		public static void Error(object message) => Error(message.ToString());

		public static void Fatal(string message) => InternalCalls.Log_Fatal(message);
		public static void Fatal(object message) => Fatal(message.ToString());
	}

}

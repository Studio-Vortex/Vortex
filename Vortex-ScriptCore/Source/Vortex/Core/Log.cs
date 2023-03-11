namespace Vortex {

	enum LogType
	{
		Trace = 0, Info, Warn, Error, Fatal
	}

	public static class Log
	{
		public static void Print(string message) => InternalCalls.Log_Message(message, LogType.Trace);
		public static void Print(object message) => Print(message.ToString());
		
		public static void Info(string message) => InternalCalls.Log_Message(message, LogType.Info);
		public static void Info(object message) => Info(message.ToString());

		public static void Warn(string message) => InternalCalls.Log_Message(message, LogType.Warn);
		public static void Warn(object message) => Warn(message.ToString());

		public static void Error(string message) => InternalCalls.Log_Message(message, LogType.Error);
		public static void Error(object message) => Error(message.ToString());

		public static void Fatal(string message) => InternalCalls.Log_Message(message, LogType.Fatal);
		public static void Fatal(object message) => Fatal(message.ToString());
	}

}

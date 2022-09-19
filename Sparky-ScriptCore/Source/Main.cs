using System;

namespace Sparky {
	
	public class Main
	{

		public float FloatVar { get; set; }

		public Main()
		{
			Console.WriteLine("Main Constructor!");
		}

		public void PrintMessage()
		{
			Console.WriteLine("Hello World from C#!");
		}

		public void PrintCustomMessage(string message)
		{
			Console.WriteLine($"C# says: {message}");
		}

	}

}
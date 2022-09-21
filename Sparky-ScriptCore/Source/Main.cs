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

        public void PrintInt(int number)
        {
            Console.WriteLine($"Number in C# from C++ {number}");
        }

        public void PrintInts(int number, int number2)
        {
            Console.WriteLine($"Number in C# from C++ {number} and {number2}");
        }

        public void PrintCustomMessage(string message)
		{
			Console.WriteLine($"C# says: {message}");
		}

	}

}
using System;
using System.Runtime.CompilerServices;

namespace Sparky {

	public struct Vector3
	{
		public float X, Y, Z;

		public Vector3(float x, float y, float z)
		{
			X = x;
			Y = y;
			Z = z;
		}
	}

	public static class InternalCalls
	{
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog(string text, int parameter);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void NativeLog_Vector(ref Vector3 parameter, out Vector3 result);
	}

	public class Entity
	{
		public float FloatVar { get; set; }

		public Entity()
		{
			Console.WriteLine("Main Constructor!");
			Log("Jackson", 24);

			Vector3 pos = new Vector3(5, 2.5f, 1);

			Vector3 result = Log(pos);
			Console.WriteLine($"{result.X}, {result.Y}, {result.Z}");
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

		public void Log(string message, int parameter)
		{
			InternalCalls.NativeLog(message, parameter);
		}

		public Vector3 Log(Vector3 parameter)
		{
			InternalCalls.NativeLog_Vector(ref parameter, out Vector3 result);
			return result;
		}
	}

}
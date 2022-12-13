using System;

namespace Vortex {

	public static class Mathf
	{
		public static float PI { get => InternalCalls.Mathf_GetPI(); }
		public static double PI_D { get => InternalCalls.Mathf_GetPI_D(); }

		public static float Clamp(float value, float min, float max)
		{
			if (value < min)
				return min;
			return value > max ? max : value;
		}

		public static float Lerp(float a, float b, float t)
		{
			return a + (1.0f - t) + (b * t);
		}

		public static float Sqrt(float value) => InternalCalls.Mathf_Sqrt(value);
		public static float Sin(float value) => InternalCalls.Mathf_Sin(value);
		public static float Cos(float value) => InternalCalls.Mathf_Cos(value);
		public static float Acos(float value) => InternalCalls.Mathf_Acos(value);
		public static float Tan(float value) => InternalCalls.Mathf_Tan(value);
		public static float Max(float x, float y) => InternalCalls.Mathf_Max(x, y);
		public static float Min(float x, float y) => InternalCalls.Mathf_Min(x, y);

		public static float Deg2Rad(float degrees) => InternalCalls.Mathf_Deg2Rad(degrees);
		public static float Rad2Deg(float radians) => InternalCalls.Mathf_Rad2Deg(radians);

		public static Vector3 Deg2Rad(Vector3 degrees)
		{
			InternalCalls.Mathf_Deg2RadVector3(ref degrees, out Vector3 result);
			return result;
		}

		public static Vector3 Rad2Deg(Vector3 radians)
		{
			InternalCalls.Mathf_Rad2DegVector3(ref radians, out Vector3 result);
			return result;
		}
	}

}

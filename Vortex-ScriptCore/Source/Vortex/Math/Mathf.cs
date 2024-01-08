using System;

namespace Vortex {

	public static class Mathf
	{
		public const float PI = (float)Math.PI;
		public const float TwoPI = PI * 2;
		public const double PI_D = Math.PI;

		public const float Deg2Rad = PI / 180.0f;
		public const float Rad2Deg = 180.0f / PI;

		public static float Abs(float value) => Math.Abs(value);
		public static float Asin(float value) => (float)Math.Asin(value);
		public static float Acos(float value) => (float)Math.Acos(value);
		public static float Atan(float value) => (float)Math.Atan(value);
		public static float Atan2(float y, float x) => (float)Math.Atan2(y, x);
		public static float Sqrt(float value) => (float)Math.Sqrt(value);
		public static float Pow(float value, float power) => (float)Math.Pow(value, power);
		public static float Sin(float value) => (float)Math.Sin(value);
		public static float Sinh(float value) => (float)Math.Sinh(value);
		public static float Cos(float value) => (float)Math.Cos(value);
		public static float Cosh(float value) => (float)Math.Cosh(value);
		public static float Tan(float value) => (float)Math.Tan(value);
		public static float Tanh(float value) => (float)Math.Tanh(value);
		public static float Sign(float value) => Math.Sign(value);
		public static float Log(float value) => (float)Math.Log(value);
		public static float Log(float value, float newBase) => (float)Math.Log(value, newBase);
		public static float Log10(float value) => (float)Math.Log10(value);
		public static float Exp(float value) => (float)Math.Exp(value);
		public static float Max(float x, float y) => Math.Max(x, y);
		public static float Min(float x, float y) => Math.Min(x, y);
		public static float Round(float value) => (float)Math.Round(value);
		public static float Floor(float value) => (float)Math.Floor(value);
		public static float Ceiling(float value) => (float)Math.Ceiling(value);
		public static float Truncate(float value) => (float)Math.Truncate(value);

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

		// Not the same as (a % b)
		public static float Modulo(float a, float b) => a - b * (float)Math.Floor(a / b);

		public static Vector3 EulerAngles(Quaternion orientation)
		{
			InternalCalls.Mathf_EulerAngles(ref orientation, out Vector3 eulers);
			return eulers;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="eyePos">the eye position of the viewer</param>
		/// <param name="worldPoint">the world point to look at</param>
		/// <returns>The final Rotation of the computed transform</returns>
		public static Quaternion LookAt(Vector3 eyePos, Vector3 worldPoint)
		{
			InternalCalls.Mathf_LookAt(ref eyePos, ref worldPoint, out Quaternion result);
			return result;
		}

		public static Quaternion Inverse(Quaternion rotation)
		{
			InternalCalls.Mathf_InverseQuat(ref rotation, out Quaternion result);
			return result;
		}
	}

}

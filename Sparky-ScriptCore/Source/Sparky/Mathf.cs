namespace Sparky {

	public static class Mathf
	{
		public static float PI
		{
			get
			{
				return InternalCalls.Mathf_GetPI();
			}
		}

		public static double PI_D
		{
			get
			{
				return InternalCalls.Mathf_GetPI_D();
			}
		}

		public static float Sin(float value)
		{
			return InternalCalls.Mathf_Sin(value);
		}

		public static float Cos(float value)
		{
			return InternalCalls.Mathf_Cos(value);
		}

		public static float Tan(float value)
		{
			return InternalCalls.Mathf_Tan(value);
		}

		public static float Max(float x, float y)
		{
			return InternalCalls.Mathf_Max(x, y);
		}

		public static float Min(float x, float y)
		{
			return InternalCalls.Mathf_Min(x, y);
		}

		public static float Deg2Rad(float degrees)
		{
			return InternalCalls.Mathf_Deg2Rad(degrees);
		}

		public static float Rad2Deg(float degrees)
		{
			return InternalCalls.Mathf_Rad2Deg(degrees);
		}
	}

}

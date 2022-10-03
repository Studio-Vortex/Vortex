namespace Sparky {

	public static class RandomNumberGenerator
	{
		public static int NewInt(int min, int max)
		{
			return InternalCalls.Random_Range_Int32(min, max);
		}

		public static float NewFloat(float min, float max)
		{
			return InternalCalls.Random_Range_Float(min, max);
		}
	}

}

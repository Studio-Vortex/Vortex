namespace Sparky {

	public static class RandomDevice
	{
		public static int NewInt(int min, int max)
		{
			return InternalCalls.RandomDevice_NewInt32(min, max);
		}

		public static float NewFloat(float min, float max)
		{
			return InternalCalls.RandomDevice_NewFloat(min, max);
		}
	}

}

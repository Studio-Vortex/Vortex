namespace Vortex {

	public static class RandomDevice
	{
		public static int RangedInt(int min, int max)
		{
			return InternalCalls.RandomDevice_RangedInt32(min, max);
		}

		public static float RangedFloat(float min, float max)
		{
			return InternalCalls.RandomDevice_RangedFloat(min, max);
		}
	}

}

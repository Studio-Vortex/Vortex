namespace Vortex {

	public static class Random
	{
		public static int Int(int min, int max)
		{
			return InternalCalls.Random_RangedInt32(min, max);
		}

		public static float Float(float min, float max)
		{
			return InternalCalls.Random_RangedFloat(min, max);
		}

		public static float Value => InternalCalls.Random_Float();
	}

}

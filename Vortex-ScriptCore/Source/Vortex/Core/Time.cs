namespace Vortex {

	public static class Time
	{
		public static float Elapsed
		{
			get
			{
				return InternalCalls.Time_GetElapsed();
			}
		}

		public static float DeltaTime
		{
			get
			{
				return InternalCalls.Time_GetDeltaTime();
			}
		}
	}

}

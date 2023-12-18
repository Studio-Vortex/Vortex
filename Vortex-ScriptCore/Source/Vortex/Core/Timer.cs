namespace Vortex {

	public class Timer
	{
		internal ulong EntityID;
		public string Name;

		internal Timer(ulong entityID, string name)
		{
			EntityID = entityID;
			Name = name;
		}

		public float TimeLeft => InternalCalls.Timer_GetTimeLeft(EntityID, Name);
		public bool Started => InternalCalls.Timer_IsStarted(EntityID, Name);
		public bool Finished => InternalCalls.Timer_IsFinished(EntityID, Name);

		public void Start() => InternalCalls.Timer_Start(EntityID, Name);

		public void Stop() => InternalCalls.Timer_Stop(EntityID, Name);
	}

}

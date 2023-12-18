namespace Vortex {

	public class Timer
	{
		internal ulong ActorID;

		internal Timer(ulong actorID, string name)
		{
			ActorID = actorID;
			Name = name;
		}

		public string Name;
		public float TimeLeft => InternalCalls.Timer_GetTimeLeft(ActorID, Name);
		public bool Started => InternalCalls.Timer_IsStarted(ActorID, Name);
		public bool Finished => InternalCalls.Timer_IsFinished(ActorID, Name);

		public void Start() => InternalCalls.Timer_Start(ActorID, Name);
	}

}

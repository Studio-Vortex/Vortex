namespace Vortex {

	public struct Collision
	{
		public ulong ActorID { get; private set; }

		public Actor Actor => Scene.FindActorByID(ActorID);

		public string Tag => Actor.Tag;
		public string Marker => Actor.Marker;
	}

}

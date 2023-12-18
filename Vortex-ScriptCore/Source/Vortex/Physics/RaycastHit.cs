namespace Vortex {

	public struct RaycastHit
	{
		public ulong EntityID { get; private set; }
		public Vector3 Position { get; private set; }
		public Vector3 Normal { get; private set; }
		public float Distance { get; private set; }

		public Actor Entity => Scene.FindActorByID(EntityID);
	}

}

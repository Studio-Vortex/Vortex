namespace Vortex {

	public struct Collision
	{
		public ulong EntityID { get; private set; }

		public Entity Entity => Scene.FindEntityByID(EntityID);

		public string Tag => Entity.Tag;
		public string Marker => Entity.Marker;
	}

}

namespace Vortex {

	public struct AudioClip
	{
		internal AudioClip(AudioSource src)
		{
			Source = src;
		}

		public AudioSource Source;

		public string Name => InternalCalls.AudioClip_GetName(Source.Entity.ID);

		public float Length => InternalCalls.AudioClip_GetLength(Source.Entity.ID);
	}

}

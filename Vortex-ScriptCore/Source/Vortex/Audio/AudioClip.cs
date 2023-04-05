namespace Vortex {

	public struct AudioClip
	{
		public readonly AudioSource Source;

		internal AudioClip(AudioSource source)
		{
			Source = source;
		}

		public string Name => InternalCalls.AudioClip_GetName(Source.Entity.ID);

		public float Length => InternalCalls.AudioClip_GetLength(Source.Entity.ID);
	}

}

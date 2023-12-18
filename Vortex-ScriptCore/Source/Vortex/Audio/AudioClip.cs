namespace Vortex {

	public struct AudioClip
	{
		public readonly AudioSource Source;

		internal AudioClip(AudioSource source)
		{
			Source = source;
		}

		public string Name => InternalCalls.AudioClip_GetName(Source.Actor.ID);

		public float Length => InternalCalls.AudioClip_GetLength(Source.Actor.ID);
	}

}

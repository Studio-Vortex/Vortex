namespace Vortex {

	public struct AudioCone
	{
		internal AudioSource Source;

		internal AudioCone(AudioSource source)
		{
			Source = source;
		}

		public float InnerAngle
		{
			get => InternalCalls.AudioSourceComponent_GetConeInnerAngle(Source.Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetConeInnerAngle(Source.Entity.ID, value);
		}

		public float OuterAngle
		{
			get => InternalCalls.AudioSourceComponent_GetConeOuterAngle(Source.Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetConeOuterAngle(Source.Entity.ID, value);
		}

		public float OuterGain
		{
			get => InternalCalls.AudioSourceComponent_GetConeOuterGain(Source.Entity.ID);
			set => InternalCalls.AudioSourceComponent_SetConeOuterGain(Source.Entity.ID, value);
		}
	}

}

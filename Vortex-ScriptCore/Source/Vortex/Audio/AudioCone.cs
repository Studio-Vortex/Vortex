namespace Vortex {

	public struct AudioCone<AudioComponent>
		where AudioComponent: Component, new()
	{
		public readonly AudioComponent Source;
		public readonly Actor Actor;

		internal AudioCone(AudioComponent data)
		{
			Source = data;
			Actor = Source.Actor;
		}

		public float InnerAngle
		{
			get => InternalCalls.AudioCone_GetInnerAngle(Actor.ID);
			set => InternalCalls.AudioCone_SetInnerAngle(Actor.ID, value);
		}

		public float OuterAngle
		{
			get => InternalCalls.AudioCone_GetOuterAngle(Actor.ID);
			set => InternalCalls.AudioCone_SetOuterAngle(Actor.ID, value);
		}

		public float OuterGain
		{
			get => InternalCalls.AudioCone_GetOuterGain(Actor.ID);
			set => InternalCalls.AudioCone_SetOuterGain(Actor.ID, value);
		}
	}

}

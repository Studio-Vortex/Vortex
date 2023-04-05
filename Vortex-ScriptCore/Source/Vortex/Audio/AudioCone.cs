namespace Vortex {

	public struct AudioCone<AudioComponent>
		where AudioComponent: Component, new()
	{
		public readonly AudioComponent Source;
		public readonly Entity Entity;

		internal AudioCone(AudioComponent data)
		{
			Source = data;
			Entity = Source.Entity;
		}

		public float InnerAngle
		{
			get => InternalCalls.AudioCone_GetInnerAngle(Entity.ID);
			set => InternalCalls.AudioCone_SetInnerAngle(Entity.ID, value);
		}

		public float OuterAngle
		{
			get => InternalCalls.AudioCone_GetOuterAngle(Entity.ID);
			set => InternalCalls.AudioCone_SetOuterAngle(Entity.ID, value);
		}

		public float OuterGain
		{
			get => InternalCalls.AudioCone_GetOuterGain(Entity.ID);
			set => InternalCalls.AudioCone_SetOuterGain(Entity.ID, value);
		}
	}

}

namespace Vortex {

	public struct PostProcessInfo
	{
		public Actor Actor;

		internal PostProcessInfo(Actor actor)
		{
			Actor = actor;
			Bloom = new BloomInfo(Actor);
		}

		public struct BloomInfo
		{
			public Actor Actor;

			public float Threshold
			{
				get => InternalCalls.BloomInfo_GetThreshold(Actor.ID);
				set => InternalCalls.BloomInfo_SetThreshold(Actor.ID, value);
			}

			public float Knee
			{
				get => InternalCalls.BloomInfo_GetKnee(Actor.ID);
				set => InternalCalls.BloomInfo_SetKnee(Actor.ID, value);
			}

			public float Intensity
			{
				get => InternalCalls.BloomInfo_GetIntensity(Actor.ID);
				set => InternalCalls.BloomInfo_SetIntensity(Actor.ID, value);
			}

			public bool Enabled
			{
				get => InternalCalls.BloomInfo_GetEnabled(Actor.ID);
				set => InternalCalls.BloomInfo_SetEnabled(Actor.ID, value);
			}

			internal BloomInfo(Actor actor)
			{
				Actor = actor;
				Threshold = Knee = Intensity = 0f;
				Enabled = false;
			}
		}

		public BloomInfo Bloom;

		public bool Enabled
		{
			get => InternalCalls.PostProcessInfo_GetEnabled(Actor.ID);
			set => InternalCalls.PostProcessInfo_SetEnabled(Actor.ID, value);
		}
	}

}

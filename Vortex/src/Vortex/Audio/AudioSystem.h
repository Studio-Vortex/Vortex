#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Audio/AudioSource.h"

namespace Vortex {

	class Scene;

	class VORTEX_API AudioSystem
	{
	public:
		static void Init();
		static void Shutdown();

		static void StartAudioSources(Scene* contextScene);
		static void PauseAudioSources(Scene* contextScene);
		static void ResumeAudioSources(Scene* contextScene);
		static void StopAudioSources(Scene* contextScene);

	private:
		inline static std::vector<SharedReference<AudioSource>> s_AudioSourcesToResume;
	};

}

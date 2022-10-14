#pragma once

#include "Sparky/Audio/AudioSource.h"

namespace Sparky {

	class AudioEngine
	{
	public:
		AudioEngine() = delete;
		AudioEngine(const AudioEngine&) = delete;

		static void Init();
		static void Shutdown();

		static void PlayFromAudioSource(const SharedRef<AudioSource>& audioSource);
		static void StartAllAudio();
		static void StopAllAudio();
	};

}

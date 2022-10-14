#include "sppch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace Sparky {

	struct AudioEngineInternalData
	{
		ma_engine Engine;
		ma_result Result;
	};

	static AudioEngineInternalData s_Data;

	void AudioEngine::Init()
	{
		s_Data.Result = ma_engine_init(nullptr, &s_Data.Engine);
		bool success = s_Data.Result == MA_SUCCESS;
		SP_CORE_ASSERT(success, "AudioEngine failed to initialize!");
	}

	void AudioEngine::Shutdown()
	{
		ma_engine_uninit(&s_Data.Engine);
	}

	void AudioEngine::PlayFromAudioSource(const AudioSource& audioSource)
	{
		ma_engine_play_sound(&s_Data.Engine, audioSource.GetPath().c_str(), nullptr);
	}

}

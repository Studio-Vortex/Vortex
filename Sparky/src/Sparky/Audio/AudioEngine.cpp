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
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "AudioEngine failed to initialize!");
	}

	void AudioEngine::Shutdown()
	{
		ma_engine_uninit(&s_Data.Engine);
	}

	void AudioEngine::PlayFromAudioSource(const SharedRef<AudioSource>& audioSource)
	{
		std::filesystem::path audioSourcePath(audioSource->GetPath());

		// If the path doesn't exist and we try to play the non-existant file the engine will crash
		if (std::filesystem::exists(audioSourcePath))
		{
			s_Data.Result = ma_engine_play_sound(&s_Data.Engine, audioSource->GetPath().c_str(), nullptr);
			SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to play audio from " + audioSource->GetPath());
		}
	}

	void AudioEngine::StartAllAudio()
	{
		s_Data.Result = ma_engine_start(&s_Data.Engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to start all audio!");
	}

	void AudioEngine::StopAllAudio()
	{
		s_Data.Result = ma_engine_stop(&s_Data.Engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to stop all audio!");
	}

}

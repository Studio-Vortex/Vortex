#include "sppch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace Sparky {

	struct AudioEngineInternalData
	{
		ma_engine AudioEngine;
		ma_result Result;
		ma_context Context;
		ma_uint32 PlaybackDeviceCount;
		ma_device_info* pPlaybackDeviceInfos;

		std::vector<ma_sound*> LoadedSounds;
	};

	static AudioEngineInternalData s_Data;

	void AudioEngine::Init()
	{
		s_Data.Result = ma_context_init(NULL, 0, NULL, &s_Data.Context);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to initialize Audio Context!");

		s_Data.Result = ma_context_get_devices(&s_Data.Context, &s_Data.pPlaybackDeviceInfos, &s_Data.PlaybackDeviceCount, nullptr, nullptr);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to retrieve Audio Device Information!");

		SP_CORE_INFO("{} Audio Devices:", s_Data.PlaybackDeviceCount);
		for (uint32_t i = 0; i < s_Data.PlaybackDeviceCount; ++i)
			SP_CORE_INFO("  {}: {}", i + 1, s_Data.pPlaybackDeviceInfos[i].name);

		// Init the audio engine
		s_Data.Result = ma_engine_init(nullptr, &s_Data.AudioEngine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "AudioEngine failed to initialize!");
	}

	void AudioEngine::Shutdown()
	{
		ma_context_uninit(&s_Data.Context);
		ma_engine_uninit(&s_Data.AudioEngine);
	}

	void AudioEngine::InitSoundFromPath(const std::string& filepath, ma_sound* sound, bool loop, float volume)
	{
		// If the path doesn't exist and we try to initialize a sound the audio engine will crash
		if (!std::filesystem::exists(filepath))
			return;

		s_Data.Result = ma_sound_init_from_file(&s_Data.AudioEngine, filepath.c_str(), 0, nullptr, nullptr, sound);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to initialize sound file from " + filepath);

		if (loop)
			ma_sound_set_looping(sound, MA_TRUE);

		ma_sound_set_volume(sound, volume);

		s_Data.LoadedSounds.push_back(sound);
	}

	void AudioEngine::DestroySound(ma_sound* sound)
	{
		if (sound != nullptr)
			ma_sound_uninit(sound);
	}

	void AudioEngine::RemoveLoadedSound(ma_sound* sound)
	{
		std::remove(s_Data.LoadedSounds.begin(), s_Data.LoadedSounds.end(), sound);
	}

	void AudioEngine::DestroyLoadedSounds()
	{
		for (auto& sound : s_Data.LoadedSounds)
			DestroySound(sound);

		s_Data.LoadedSounds.clear();
	}

	void AudioEngine::PlayFromSound(ma_sound* sound)
	{
		if (sound != nullptr)
			ma_sound_start(sound);
	}

	void AudioEngine::StopSound(ma_sound* sound)
	{
		if (sound != nullptr)
		{
			ma_sound_stop(sound);
			ma_sound_uninit(sound);
		}
	}

	void AudioEngine::SetVolume(ma_sound* sound, float volume)
	{
		ma_sound_set_volume(sound, volume);
	}

	bool AudioEngine::IsPlaying(ma_sound* sound)
	{
		return ma_sound_is_playing(sound);
	}

	void AudioEngine::StartEngine()
	{
		s_Data.Result = ma_engine_start(&s_Data.AudioEngine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to start all audio!");
	}

	void AudioEngine::StopEngine()
	{
		s_Data.Result = ma_engine_stop(&s_Data.AudioEngine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to stop all audio!");
	}

}

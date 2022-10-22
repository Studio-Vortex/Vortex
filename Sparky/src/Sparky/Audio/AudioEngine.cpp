#include "sppch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

namespace Sparky {

	struct AudioEngineInternalData
	{
		ma_result Result;
		ma_context Context;
		ma_uint32 PlaybackDeviceCount;
		ma_device_info* pPlaybackDeviceInfos;
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
	}

	void AudioEngine::Shutdown()
	{
		ma_context_uninit(&s_Data.Context);
	}

	void AudioEngine::InitEngine(ma_engine* engine)
	{
		// Init the audio engine
		s_Data.Result = ma_engine_init(nullptr, engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to initialize Audio Engine!");
	}

	void AudioEngine::ShutdownEngine(ma_engine* engine)
	{
		ma_engine_uninit(engine);
	}

	void AudioEngine::InitSoundFromPath(ma_engine* preInitializedEngine, const std::string& filepath, ma_sound* sound, bool loop, bool spacialized, float volume)
	{
		// If the path doesn't exist and we try to initialize a sound the audio engine will crash
		if (!std::filesystem::exists(filepath))
			return;

		s_Data.Result = ma_sound_init_from_file(preInitializedEngine, filepath.c_str(), 0, nullptr, nullptr, sound);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to initialize sound file from " + filepath);

		if (loop)
			ma_sound_set_looping(sound, MA_TRUE);
		else
			ma_sound_set_looping(sound, MA_FALSE);

		if (spacialized)
			ma_sound_set_spatialization_enabled(sound, MA_TRUE);
		else
			ma_sound_set_spatialization_enabled(sound, MA_FALSE);

		ma_sound_set_volume(sound, volume);
	}

	void AudioEngine::DestroySound(ma_sound* sound)
	{
		if (sound != nullptr)
			ma_sound_uninit(sound);
	}

	void AudioEngine::PlayFromSound(ma_sound* sound)
	{
		if (sound != nullptr)
			ma_sound_start(sound);
	}

	void AudioEngine::RestartSound(ma_sound* sound)
	{
		if (sound != nullptr)
			// Seek to the beginning of the audio file
			ma_sound_seek_to_pcm_frame(sound, 0);
	}

	void AudioEngine::StopSound(ma_sound* sound)
	{
		if (sound != nullptr)
		{
			ma_sound_stop(sound);
			ma_sound_uninit(sound);
		}
	}

	void AudioEngine::SetPosition(ma_sound* sound, const Math::vec3& position)
	{
		ma_sound_set_position(sound, position.x, position.y, position.z);
	}

	void AudioEngine::SetDirection(ma_sound* sound, const Math::vec3& direction)
	{
		ma_sound_set_direction(sound, direction.x, direction.y, direction.z);
	}

	void AudioEngine::SetVeloctiy(ma_sound* sound, const Math::vec3& veloctiy)
	{
		ma_sound_set_velocity(sound, veloctiy.x, veloctiy.y, veloctiy.z);
	}
	
	void AudioEngine::SetCone(ma_sound* sound, float innerAngleRadians, float outerAngleRadians, float outerGain)
	{
		ma_sound_set_cone(sound, innerAngleRadians, outerAngleRadians, outerGain);
	}

	void AudioEngine::SetMinDistance(ma_sound* sound, float minDistance)
	{
		ma_sound_set_min_distance(sound, minDistance);
	}

	void AudioEngine::SetMaxDistance(ma_sound* sound, float maxDistance)
	{
		ma_sound_set_max_distance(sound, maxDistance);
	}

	void AudioEngine::SetPitch(ma_sound* sound, float pitch)
	{
		ma_sound_set_pitch(sound, pitch);
	}

	void AudioEngine::SetDopplerFactor(ma_sound* sound, float dopplerFactor)
	{
		ma_sound_set_doppler_factor(sound, dopplerFactor);
	}

	void AudioEngine::SetVolume(ma_sound* sound, float volume)
	{
		ma_sound_set_volume(sound, volume);
	}

	void AudioEngine::SetSpacialized(ma_sound* sound, bool spacialized)
	{
		ma_sound_set_spatialization_enabled(sound, spacialized);
	}

	void AudioEngine::SetLoop(ma_sound* sound, bool loop)
	{
		ma_sound_set_looping(sound, loop);
	}

	bool AudioEngine::IsPlaying(ma_sound* sound)
	{
		return ma_sound_is_playing(sound);
	}

	void AudioEngine::StartEngine(ma_engine* engine)
	{
		s_Data.Result = ma_engine_start(engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to start Audio Engine!");
	}

	void AudioEngine::StopEngine(ma_engine* engine)
	{
		s_Data.Result = ma_engine_stop(engine);
		SP_CORE_ASSERT(s_Data.Result == MA_SUCCESS, "Failed to stop Audio Engine!");
	}

}

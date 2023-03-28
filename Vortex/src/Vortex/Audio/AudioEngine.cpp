#include "vxpch.h"
#include "AudioEngine.h"

#include "Vortex/Utils/FileSystem.h"

#include <miniaudio/miniaudio.h>

namespace Vortex {

#define VX_CHECK_AUDIO_RESULT(res, msg, ...) { if((res) != MA_SUCCESS) { VX_CORE_ASSERT(false, msg, __VA_ARGS__) } } 

	void AudioEngine::InitEngine(ma_engine* engine)
	{
		// Init the engine
		VX_CHECK_AUDIO_RESULT(ma_engine_init(nullptr, engine), "Failed to initialize Audio Engine!");
	}

	void AudioEngine::ShutdownEngine(ma_engine* engine)
	{
		ma_engine_uninit(engine);
	}

	void AudioEngine::InitSoundFromPath(ma_engine* preInitializedEngine, const std::string& filepath, ma_sound* sound, float* length)
	{
		if (!FileSystem::Exists(filepath))
		{
			VX_CORE_ASSERT(false, "Cannot load sound that doesn't exist!");
			return;
		}

		VX_CHECK_AUDIO_RESULT(
			ma_sound_init_from_file(
				preInitializedEngine,
				filepath.c_str(),
				MA_SOUND_FLAG_ASYNC,
				nullptr, nullptr,
				sound
			),
			"Failed to initialize sound file from {}", filepath
		);

		ma_sound_get_length_in_seconds(sound, length);
	}

	void AudioEngine::DestroySound(ma_sound* sound)
	{
		if (!sound)
			return;

		ma_sound_uninit(sound);
	}

	void AudioEngine::PlayFromSound(ma_sound* sound)
	{
		if (!sound)
			return;

		ma_sound_start(sound);
	}

	void AudioEngine::PlayOneShot(ma_engine* engine, const char* filepath)
	{
		if (!engine || strlen(filepath) == 0)
			return;

		ma_engine_play_sound(engine, filepath, nullptr);
	}

    void AudioEngine::PauseSound(ma_sound* sound)
    {
		ma_sound_stop(sound);
    }

	void AudioEngine::RestartSound(ma_sound* sound)
	{
		if (!sound)
			return;

		// seek to beggining of track, sound is still playing at this point
		ma_sound_seek_to_pcm_frame(sound, 0);
	}

	void AudioEngine::StopSound(ma_sound* sound)
	{
		if (!sound)
			return;
		
		ma_sound_stop(sound);
		ma_sound_uninit(sound);
	}

	void AudioEngine::SetPosition(ma_sound* sound, const Math::vec3& position)
	{
		ma_sound_set_position(sound, position.x, position.y, position.z);
	}

	void AudioEngine::SetDirection(ma_sound* sound, const Math::vec3& direction)
	{
		ma_sound_set_direction(sound, direction.x, direction.y, direction.z);
	}

	void AudioEngine::SetListenerPosition(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& position)
	{
		ma_engine_listener_set_position(engine, listenerIndex, position.x, position.y, position.z);
	}

	void AudioEngine::SetListenerDirection(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& direction)
	{
		ma_engine_listener_set_direction(engine, listenerIndex, direction.x, direction.y, direction.z);
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

	void AudioEngine::SetLooping(ma_sound* sound, bool loop)
	{
		ma_sound_set_looping(sound, loop);
	}

	float AudioEngine::GetSoundCursor(ma_sound* sound)
	{
		float cursor = 0.0f;
		ma_sound_get_cursor_in_seconds(sound, &cursor);
		return cursor;
	}

	bool AudioEngine::IsPlaying(ma_sound* sound)
	{
		return (bool)ma_sound_is_playing(sound);
	}

	void AudioEngine::StartEngine(ma_engine* engine)
	{
		VX_CHECK_AUDIO_RESULT(ma_engine_start(engine), "Failed to start Audio Engine!");
	}

	void AudioEngine::StopEngine(ma_engine* engine)
	{
		VX_CHECK_AUDIO_RESULT(ma_engine_stop(engine), "Failed to stop Audio Engine!");
	}

}

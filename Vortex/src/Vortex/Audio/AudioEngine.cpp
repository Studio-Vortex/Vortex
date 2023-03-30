#include "vxpch.h"
#include "AudioEngine.h"

#include "Vortex/Audio/AudioAssert.h"

#include "Vortex/Utils/FileSystem.h"

#include <miniaudio/miniaudio.h>

namespace Vortex {

	void AudioEngine::InitEngine(ma_engine* engine)
	{
		VX_CHECK_AUDIO_RESULT(
			ma_engine_init(nullptr, engine),
			"Failed to initialize Audio Engine!"
		);

		VX_CORE_ASSERT(engine, "Failed to create Audio Engine!");
	}

	void AudioEngine::ShutdownEngine(ma_engine* engine)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		ma_engine_uninit(engine);
	}

	void AudioEngine::InitSoundFromPath(ma_engine* initializedEngine, const std::string& filepath, ma_sound* sound, float* length)
	{
		VX_CORE_ASSERT(FileSystem::Exists(filepath), "Cannot load audio track that doesn't exist!");

		VX_CHECK_AUDIO_RESULT(
			ma_sound_init_from_file(
				initializedEngine,
				filepath.c_str(),
				MA_SOUND_FLAG_ASYNC,
				nullptr, nullptr,
				sound
			),
			"Failed to initialize audio source file from {}", filepath
		);

		VX_CHECK_AUDIO_RESULT(
			ma_sound_get_length_in_seconds(sound, length),
			"Failed to get audio track length!"
		);
	}

	void AudioEngine::DestroySound(ma_sound* sound)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");

		if (!sound)
			return;

		ma_sound_uninit(sound);
	}

	void AudioEngine::PlayFromSound(ma_sound* sound)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");

		if (!sound)
			return;

		VX_CHECK_AUDIO_RESULT(ma_sound_start(sound), "Failed to start audio track!");
	}

	void AudioEngine::PlayOneShot(ma_engine* engine, const char* filepath)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");

		if (strlen(filepath) == 0)
		{
			VX_CORE_ASSERT(false, "Trying to play audio track from invalid path!");
			return;
		}

		VX_CHECK_AUDIO_RESULT(
			ma_engine_play_sound(engine, filepath, nullptr),
			"Failed to play audio track!"
		);
	}

    void AudioEngine::PauseSound(ma_sound* sound)
    {
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		VX_CHECK_AUDIO_RESULT(ma_sound_stop(sound), "Failed to stop audio track!");
    }

	void AudioEngine::RestartSound(ma_sound* sound)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");

		if (!sound)
			return;

		VX_CHECK_AUDIO_RESULT(
			ma_sound_seek_to_pcm_frame(sound, 0),
			"Failed to seek to beginning of audio track!"
		);
	}

	void AudioEngine::StopSound(ma_sound* sound)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");

		if (!sound)
			return;
		
		VX_CHECK_AUDIO_RESULT(ma_sound_stop(sound), "Failed to stop audio track!");
		ma_sound_uninit(sound);
	}

	float AudioEngine::GetSoundCursor(ma_sound* sound)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");

		float cursor = 0.0f;

		VX_CHECK_AUDIO_RESULT(
			ma_sound_get_cursor_in_seconds(sound, &cursor),
			"Failed to get cursor!"
		);
		
		return cursor;
	}

	bool AudioEngine::IsSoundPlaying(ma_sound* sound)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		return (bool)ma_sound_is_playing(sound);
	}

	void AudioEngine::SetSoundPosition(ma_sound* sound, const Math::vec3& position)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_position(sound, position.x, position.y, position.z);
	}

	void AudioEngine::SetSoundDirection(ma_sound* sound, const Math::vec3& direction)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_direction(sound, direction.x, direction.y, direction.z);
	}

	void AudioEngine::SetSoundVeloctiy(ma_sound* sound, const Math::vec3& veloctiy)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_velocity(sound, veloctiy.x, veloctiy.y, veloctiy.z);
	}
	
	void AudioEngine::SetSoundCone(ma_sound* sound, float innerAngleRadians, float outerAngleRadians, float outerGain)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_cone(sound, innerAngleRadians, outerAngleRadians, outerGain);
	}

	void AudioEngine::SetSoundMinGain(ma_sound* sound, float minGain)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_min_gain(sound, minGain);
	}

	void AudioEngine::SetSoundMaxGain(ma_sound* sound, float maxGain)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_max_gain(sound, maxGain);
	}

	void AudioEngine::SetSoundAttenuationModel(ma_sound* sound, AttenuationModel attenuationModel)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_attenuation_model(sound, (ma_attenuation_model)attenuationModel);
	}

	void AudioEngine::SetSoundFalloff(ma_sound* sound, float falloff)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_rolloff(sound, falloff);
	}

	void AudioEngine::SetSoundMinDistance(ma_sound* sound, float minDistance)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_min_distance(sound, minDistance);
	}

	void AudioEngine::SetSoundMaxDistance(ma_sound* sound, float maxDistance)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_max_distance(sound, maxDistance);
	}

	void AudioEngine::SetSoundPitch(ma_sound* sound, float pitch)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_pitch(sound, pitch);
	}

	void AudioEngine::SetSoundDopplerFactor(ma_sound* sound, float dopplerFactor)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_doppler_factor(sound, dopplerFactor);
	}

	void AudioEngine::SetSoundVolume(ma_sound* sound, float volume)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_volume(sound, volume);
	}

	void AudioEngine::SetSoundSpacialized(ma_sound* sound, bool spacialized)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_spatialization_enabled(sound, spacialized);
	}

	void AudioEngine::SetSoundLooping(ma_sound* sound, bool loop)
	{
		VX_CORE_ASSERT(sound, "Invalid Sound!");
		ma_sound_set_looping(sound, loop);
	}

	uint32_t AudioEngine::GetDeviceListenerCount(ma_engine* engine)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		return ma_engine_get_listener_count(engine);
	}

	void AudioEngine::AddDeviceListener(ma_engine* engine, uint32_t listenerIndex)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		ma_engine_listener_set_enabled(engine, listenerIndex, (ma_bool32)true);
	}

	void AudioEngine::RemoveDeviceListener(ma_engine* engine, uint32_t listenerIndex)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		ma_engine_listener_set_enabled(engine, listenerIndex, (ma_bool32)false);
	}

	bool AudioEngine::IsListenerEnabled(ma_engine* engine, uint32_t listenerIndex)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		return ma_engine_listener_is_enabled(engine, listenerIndex);
	}

	void AudioEngine::SetListenerPosition(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& position)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		VX_CORE_ASSERT(IsListenerEnabled(engine, listenerIndex), "Listener not enabled!");
		ma_engine_listener_set_position(engine, listenerIndex, position.x, position.y, position.z);
	}

	void AudioEngine::SetListenerDirection(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& direction)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		VX_CORE_ASSERT(IsListenerEnabled(engine, listenerIndex), "Listener not enabled!");
		ma_engine_listener_set_direction(engine, listenerIndex, direction.x, direction.y, direction.z);
	}

	void AudioEngine::SetListenerVeloctiy(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& veloctiy)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		VX_CORE_ASSERT(IsListenerEnabled(engine, listenerIndex), "Listener not enabled!");
		ma_engine_listener_set_velocity(engine, listenerIndex, veloctiy.x, veloctiy.y, veloctiy.z);
	}

	void AudioEngine::SetListenerCone(ma_engine* engine, uint32_t listenerIndex, float innerAngleRadians, float outerAngleRadians, float outerGain)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		VX_CORE_ASSERT(IsListenerEnabled(engine, listenerIndex), "Listener not enabled!");
		ma_engine_listener_set_cone(engine, listenerIndex, innerAngleRadians, outerAngleRadians, outerGain);
	}

	void AudioEngine::SetListenerWorldUp(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& up)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		VX_CORE_ASSERT(IsListenerEnabled(engine, listenerIndex), "Listener not enabled!");
		ma_engine_listener_set_world_up(engine, listenerIndex, up.x, up.y, up.z);
	}

	void AudioEngine::StartEngine(ma_engine* engine)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		VX_CHECK_AUDIO_RESULT(ma_engine_start(engine), "Failed to start Audio Engine!");
	}

	void AudioEngine::StopEngine(ma_engine* engine)
	{
		VX_CORE_ASSERT(engine, "Invalid Audio Engine!");
		VX_CHECK_AUDIO_RESULT(ma_engine_stop(engine), "Failed to stop Audio Engine!");
	}

}

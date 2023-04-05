#include "vxpch.h"
#include "PlaybackDevice.h"

#include "Vortex/Audio/AudioEngine.h"

namespace Vortex {

	PlaybackDevice::PlaybackDevice()
	{
		AudioEngine::InitEngine(&m_Engine);
	}
	
	PlaybackDevice::~PlaybackDevice()
	{
		AudioEngine::DestroySound(&m_Sound);
		AudioEngine::ShutdownEngine(&m_Engine);
	}
	
	bool PlaybackDevice::Load(const std::string& filepath, float* length)
	{
		AudioEngine::InitSoundFromPath(&m_Engine, filepath, &m_Sound, length);

		return (bool)(&m_Sound && &m_Engine);
	}

	void PlaybackDevice::Play()
	{
		AudioEngine::StartEngine(&m_Engine);
		AudioEngine::PlayFromSound(&m_Sound);
		m_IsPaused = false;
	}

	void PlaybackDevice::PlayOneShot(const std::string& filepath)
	{
		AudioEngine::StartEngine(&m_Engine);
		AudioEngine::PlayOneShot(&m_Engine, filepath.c_str());
		// Engine will be shutdown automatically
	}

	void PlaybackDevice::Pause()
	{
		AudioEngine::PauseSound(&m_Sound);
		m_IsPaused = true;
	}

	void PlaybackDevice::Restart()
	{
		AudioEngine::RestartSound(&m_Sound);
		m_IsPaused = false;
	}

	void PlaybackDevice::Stop()
	{
		AudioEngine::StopSound(&m_Sound);
		m_IsPaused = false;
		AudioEngine::StopEngine(&m_Engine);
	}

	bool PlaybackDevice::IsPlaying() const
	{
		return AudioEngine::IsSoundPlaying(&m_Sound);
	}

    bool PlaybackDevice::IsPaused() const
    {
        return m_IsPaused;
    }

	float PlaybackDevice::GetSoundCursor() const
	{
		return AudioEngine::GetSoundCursor(&m_Sound);
	}

	void PlaybackDevice::SetPosition(const Math::vec3& position)
	{
		AudioEngine::SetSoundPosition(&m_Sound, position);
	}

	void PlaybackDevice::SetDirection(const Math::vec3& direction)
	{
		AudioEngine::SetSoundDirection(&m_Sound, direction);
	}

	void PlaybackDevice::SetVelocity(const Math::vec3& velocity)
	{
		AudioEngine::SetSoundVeloctiy(&m_Sound, velocity);
	}

	void PlaybackDevice::SetCone(float innerAngleRad, float outerAngleRad, float outerGain)
	{
		AudioEngine::SetSoundCone(&m_Sound, innerAngleRad, outerAngleRad, outerGain);
	}

	void PlaybackDevice::SetMinGain(float minGain)
	{
		AudioEngine::SetSoundMinGain(&m_Sound, minGain);
	}

	void PlaybackDevice::SetMaxGain(float maxGain)
	{
		AudioEngine::SetSoundMaxGain(&m_Sound, maxGain);
	}

	void PlaybackDevice::SetAttenuationModel(AttenuationModel attenuationModel)
	{
		AudioEngine::SetSoundAttenuationModel(&m_Sound, attenuationModel);
	}

	void PlaybackDevice::SetFalloff(float falloff)
	{
		AudioEngine::SetSoundFalloff(&m_Sound, falloff);
	}

	void PlaybackDevice::SetMinDistance(float minDistance)
	{
		AudioEngine::SetSoundMinDistance(&m_Sound, minDistance);
	}

	void PlaybackDevice::SetMaxDistance(float maxDistance)
	{
		AudioEngine::SetSoundMaxDistance(&m_Sound, maxDistance);
	}

	void PlaybackDevice::SetPitch(float pitch)
	{
		AudioEngine::SetSoundPitch(&m_Sound, pitch);
	}

	void PlaybackDevice::SetDopplerFactor(float dopplerFactor)
	{
		AudioEngine::SetSoundDopplerFactor(&m_Sound, dopplerFactor);
	}

	void PlaybackDevice::SetVolume(float volume)
	{
		AudioEngine::SetSoundVolume(&m_Sound, volume);
	}

	void PlaybackDevice::SetSpacialized(bool spacialized)
	{
		AudioEngine::SetSoundSpacialized(&m_Sound, spacialized);
	}

	void PlaybackDevice::SetLooping(bool looping)
	{
		AudioEngine::SetSoundLooping(&m_Sound, looping);
	}

	uint8_t PlaybackDevice::GetDeviceListenerCount() const
	{
		return AudioEngine::GetDeviceListenerCount(&m_Engine);
	}

	void PlaybackDevice::AddDeviceListener(uint8_t listenerIndex)
	{
		VX_CORE_ASSERT(listenerIndex < MaxDeviceListeners - 1, "Listener Index out of bounds!");
		AudioEngine::AddDeviceListener(&m_Engine, listenerIndex);
	}

	void PlaybackDevice::RemoveDeviceListener(uint8_t listenerIndex)
	{
		VX_CORE_ASSERT(listenerIndex < MaxDeviceListeners - 1, "Listener Index out of bounds!");
		AudioEngine::RemoveDeviceListener(&m_Engine, listenerIndex);
	}

	ma_engine* PlaybackDevice::GetEngine()
	{
		return &m_Engine;
	}

	ma_sound* PlaybackDevice::GetSound()
	{
		return &m_Sound;
	}

}

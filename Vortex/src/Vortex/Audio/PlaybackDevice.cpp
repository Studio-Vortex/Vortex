#include "vxpch.h"
#include "PlaybackDevice.h"

#include "Vortex/Audio/AudioEngine.h"

namespace Vortex {

	bool PlaybackDevice::Init(const std::string& filepath, float* length)
	{
		AudioEngine::InitEngine(&m_Engine);
		AudioEngine::InitSoundFromPath(&m_Engine, filepath, &m_Sound, length);

		return true;
	}

	void PlaybackDevice::Shutdown()
	{
		DestroySound();
		ShutdownEngine();
	}

	void PlaybackDevice::Play()
	{
		StartEngine();
		PlaySound();
	}

	void PlaybackDevice::PlayOneShot(const std::string& filepath)
	{
		StartEngine();
		PlayOneShotFromFilepath(filepath);
		// Engine will be shutdown automatically
	}

	void PlaybackDevice::Pause()
	{
		PauseSound();
	}

	void PlaybackDevice::Restart()
	{
		RestartSound();
	}

	void PlaybackDevice::Stop()
	{
		StopSound();
		ShutdownEngine();
	}

	float PlaybackDevice::GetSoundCursor()
	{
		return AudioEngine::GetSoundCursor(&m_Sound);
	}

	bool PlaybackDevice::IsPlaying()
	{
		return AudioEngine::IsPlaying(&m_Sound);
	}

	void PlaybackDevice::SetPosition(const Math::vec3& position)
	{
		AudioEngine::SetPosition(&m_Sound, position);
	}

	void PlaybackDevice::SetDirection(const Math::vec3& direction)
	{
		AudioEngine::SetDirection(&m_Sound, direction);
	}

	void PlaybackDevice::SetVelocity(const Math::vec3& velocity)
	{
		AudioEngine::SetVeloctiy(&m_Sound, velocity);
	}

	void PlaybackDevice::SetCone(float innerAngleRad, float outerAngleRad, float outerGain)
	{
		AudioEngine::SetCone(&m_Sound, innerAngleRad, outerAngleRad, outerGain);
	}

	void PlaybackDevice::SetMinDistance(float minDistance)
	{
		AudioEngine::SetMinDistance(&m_Sound, minDistance);
	}

	void PlaybackDevice::SetMaxDistance(float maxDistance)
	{
		AudioEngine::SetMaxDistance(&m_Sound, maxDistance);
	}

	void PlaybackDevice::SetPitch(float pitch)
	{
		AudioEngine::SetPitch(&m_Sound, pitch);
	}

	void PlaybackDevice::SetDopplerFactor(float dopplerFactor)
	{
		AudioEngine::SetDopplerFactor(&m_Sound, dopplerFactor);
	}

	void PlaybackDevice::SetVolume(float volume)
	{
		AudioEngine::SetVolume(&m_Sound, volume);
	}

	void PlaybackDevice::SetSpacialized(bool spacialized)
	{
		AudioEngine::SetSpacialized(&m_Sound, spacialized);
	}

	void PlaybackDevice::SetLooping(bool looping)
	{
		AudioEngine::SetLooping(&m_Sound, looping);
	}

	ma_engine* PlaybackDevice::GetEngine()
	{
		return &m_Engine;
	}

	ma_sound* PlaybackDevice::GetSound()
	{
		return &m_Sound;
	}

	void PlaybackDevice::PlaySound()
	{
		AudioEngine::PlayFromSound(&m_Sound);
	}

	void PlaybackDevice::PlayOneShotFromFilepath(const std::string& filepath)
	{
		AudioEngine::PlayOneShot(&m_Engine, filepath.c_str());
	}

	void PlaybackDevice::PauseSound()
	{
		AudioEngine::PauseSound(&m_Sound);
	}

	void PlaybackDevice::RestartSound()
	{
		AudioEngine::RestartSound(&m_Sound);
	}

	void PlaybackDevice::StopSound()
	{
		AudioEngine::StopSound(&m_Sound);
	}

	void PlaybackDevice::DestroySound()
	{
		AudioEngine::DestroySound(&m_Sound);
	}

	void PlaybackDevice::StartEngine()
	{
		AudioEngine::StartEngine(&m_Engine);
	}

	void PlaybackDevice::ShutdownEngine()
	{
		AudioEngine::ShutdownEngine(&m_Engine);
	}

}

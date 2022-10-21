#include "sppch.h"
#include "AudioSource.h"

#include "Sparky/Audio/AudioEngine.h"

namespace Sparky {

	AudioSource::AudioSource(const std::string& filepath)
		: m_Path(filepath)
	{
		AudioEngine::InitSoundFromPath(m_Path, &m_Sound, m_Properties.Loop, m_Properties.Spacialized, m_Properties.Volume);
		m_Initialized = true;
	}

	void AudioSource::Play()
	{
		if (!m_Initialized)
		{
			AudioEngine::InitSoundFromPath(m_Path, &m_Sound, m_Properties.Loop, m_Properties.Spacialized, m_Properties.Volume);
			m_Initialized = true;
		}

		if (IsPlaying())
			return;

		AudioEngine::PlayFromSound(&m_Sound);
	}

	void AudioSource::Stop()
	{
		if (m_Initialized)
		{
			AudioEngine::StopSound(&m_Sound);
			m_Initialized = false;
			AudioEngine::RemoveLoadedSound(&m_Sound);
		}
	}

	void AudioSource::Destroy()
	{
		AudioEngine::RemoveLoadedSound(&m_Sound);
		AudioEngine::DestroySound(&m_Sound);
	}

	bool AudioSource::IsPlaying()
	{
		return AudioEngine::IsPlaying(&m_Sound);
	}

	void AudioSource::SetPosition(const Math::vec3& position)
	{
		AudioEngine::SetPosition(&m_Sound, position);
	}

	void AudioSource::SetVolume(float volume)
	{
		AudioEngine::SetVolume(&m_Sound, volume);
	}

	void AudioSource::SetSpacialized(bool spacialized)
	{
		AudioEngine::SetSpacialized(&m_Sound, spacialized);
	}

	void AudioSource::SetLoop(bool loop)
	{
		AudioEngine::SetLoop(&m_Sound, loop);
	}

}

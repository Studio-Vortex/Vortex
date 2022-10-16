#include "sppch.h"
#include "AudioSource.h"

#include "Sparky/Audio/AudioEngine.h"

namespace Sparky {

	AudioSource::AudioSource(const std::string& filepath)
		: m_Path(filepath)
	{
		AudioEngine::InitSoundFromPath(m_Path, &m_Sound, m_Properties.Loop, m_Properties.Volume);
		m_Initialized = true;
	}

    AudioSource::~AudioSource()
    {
		Destroy();
    }

	void AudioSource::Destroy()
	{
		AudioEngine::DestroySound(&m_Sound);
	}

	void AudioSource::Play()
	{
		if (!m_Initialized)
		{
			AudioEngine::InitSoundFromPath(m_Path, &m_Sound, m_Properties.Loop, m_Properties.Volume);
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
		}
	}

	bool AudioSource::IsPlaying()
	{
		return AudioEngine::IsPlaying(&m_Sound);
	}

	void AudioSource::SetVolume(float volume)
	{
		AudioEngine::SetVolume(&m_Sound, volume);
	}

}

#include "sppch.h"
#include "AudioSource.h"

#include "Sparky/Audio/AudioEngine.h"

namespace Sparky {

	AudioSource::AudioSource(const std::string& filepath)
		: m_Path(filepath)
	{
		AudioEngine::InitEngine(&m_Engine);
		AudioEngine::InitSoundFromPath(&m_Engine, m_Path, &m_Sound, &m_LengthInSeconds, m_Properties.Loop, m_Properties.Spacialized, m_Properties.Volume);
		m_Initialized = true;
		AudioEngine::StopEngine(&m_Engine);
	}

	AudioSource::~AudioSource()
	{
		Destroy();
	}

	void AudioSource::Play()
	{
		if (!m_Initialized)
		{
			AudioEngine::InitEngine(&m_Engine);
			AudioEngine::InitSoundFromPath(&m_Engine, m_Path, &m_Sound, &m_LengthInSeconds, m_Properties.Loop, m_Properties.Spacialized, m_Properties.Volume);
			m_Initialized = true;
		}

		if (m_Properties.PlayOneShot)
		{
			AudioEngine::StartEngine(&m_Engine);
			AudioEngine::PlayOneShot(&m_Engine, m_Path.c_str());
		}
		else
		{
			AudioEngine::StartEngine(&m_Engine);
			AudioEngine::PlayFromSound(&m_Sound);
		}
	}

	void AudioSource::Pause()
	{
		if (m_Initialized)
			AudioEngine::PauseSound(&m_Sound);
	}

	void AudioSource::Restart()
	{
		if (m_Initialized)
			AudioEngine::RestartSound(&m_Sound);
	}

	void AudioSource::Stop()
	{
		if (m_Initialized)
		{
			AudioEngine::RestartSound(&m_Sound);
			AudioEngine::StopSound(&m_Sound);
			AudioEngine::ShutdownEngine(&m_Engine);
			m_Initialized = false;
		}
	}

	void AudioSource::Destroy()
	{
		if (m_Initialized)
		{
			AudioEngine::DestroySound(&m_Sound);
			AudioEngine::ShutdownEngine(&m_Engine);
		}
	}

	bool AudioSource::IsPlaying()
	{
		return AudioEngine::IsPlaying(&m_Sound);
	}

	void AudioSource::SetPosition(const Math::vec3& position)
	{
		AudioEngine::SetPosition(&m_Sound, position);
	}

	void AudioSource::SetDirection(const Math::vec3& direction)
	{
		AudioEngine::SetDirection(&m_Sound, direction);
	}

	void AudioSource::SetVelocity(const Math::vec3& velocity)
	{
		AudioEngine::SetVeloctiy(&m_Sound, velocity);
	}

	void AudioSource::SetCone(const SoundProperties::AudioCone& cone)
	{
		AudioEngine::SetCone(&m_Sound, cone.InnerAngle, cone.OuterAngle, cone.OuterGain);
	}

	void AudioSource::SetMinDistance(float minDistance)
	{
		AudioEngine::SetMinDistance(&m_Sound, minDistance);
	}

	void AudioSource::SetMaxDistance(float maxDistance)
	{
		AudioEngine::SetMaxDistance(&m_Sound, maxDistance);
	}

	void AudioSource::SetPitch(float pitch)
	{
		AudioEngine::SetPitch(&m_Sound, pitch);
	}

	void AudioSource::SetDopplerFactor(float dopplerFactor)
	{
		AudioEngine::SetDopplerFactor(&m_Sound, dopplerFactor);
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

    void AudioSource::SetPlayOnStart(bool playOnStart)
    {
		m_Properties.PlayOnStart = playOnStart;
    }

	void AudioSource::SetPlayOneShot(bool playOneShot)
	{
		m_Properties.PlayOneShot = playOneShot;
	}

	float AudioSource::GetAmountComplete()
	{
		return AudioEngine::GetSoundCursor(&m_Sound) / m_LengthInSeconds;
	}

	void AudioSource::Copy(const SharedRef<AudioSource>& dstAudioSource, const SharedRef<AudioSource>& srcAudioSource)
	{
		const auto& props = srcAudioSource->GetProperties();
		dstAudioSource->SetCone(props.Cone);
		dstAudioSource->SetDirection(props.Direction);
		dstAudioSource->SetDopplerFactor(props.DopplerFactor);
		dstAudioSource->SetLoop(props.Loop);
		dstAudioSource->SetMaxDistance(props.MaxDistance);
		dstAudioSource->SetMinDistance(props.MinDistance);
		dstAudioSource->SetPitch(props.Pitch);
		dstAudioSource->SetPosition(props.Position);
		dstAudioSource->SetSpacialized(props.Spacialized);
		dstAudioSource->SetPlayOnStart(props.PlayOnStart);
		dstAudioSource->SetPlayOneShot(props.PlayOneShot);
		dstAudioSource->SetVelocity(props.Veloctiy);
		dstAudioSource->SetVolume(props.Volume);
	}

	SharedRef<AudioSource> AudioSource::Create(const std::string& filepath)
	{
		return CreateShared<AudioSource>(filepath);
	}

}

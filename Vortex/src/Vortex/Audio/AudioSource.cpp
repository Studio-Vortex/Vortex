#include "vxpch.h"
#include "AudioSource.h"

#include "Vortex/Audio/AudioEngine.h"

namespace Vortex {

	AudioSource::AudioSource(const std::string& filepath)
		: m_Path(filepath)
	{
		Reload();
	}

	AudioSource::~AudioSource()
	{
		Destroy();
	}

	void AudioSource::Play()
	{
		if (m_Properties.PlayOneShot)
		{
			AudioEngine::StartEngine(&m_Engine);
			AudioEngine::PlayOneShot(&m_Engine, m_Path.c_str());
			return;
		}

		if (!m_Initialized)
		{
			Reload();
		}

		AudioEngine::StartEngine(&m_Engine);
		AudioEngine::PlayFromSound(&m_Sound);
	}

	void AudioSource::Pause()
	{
		if (!m_Initialized)
			return;

		AudioEngine::PauseSound(&m_Sound);
	}

	void AudioSource::Restart()
	{
		if (!m_Initialized)
			return;

		AudioEngine::RestartSound(&m_Sound);
	}

	void AudioSource::Stop()
	{
		if (!m_Initialized)
			return;

		AudioEngine::RestartSound(&m_Sound);
		AudioEngine::StopSound(&m_Sound);
		AudioEngine::ShutdownEngine(&m_Engine);
		m_Initialized = false;
	}

	void AudioSource::Destroy()
	{
		if (!m_Initialized)
			return;
		
		AudioEngine::DestroySound(&m_Sound);
		AudioEngine::ShutdownEngine(&m_Engine);
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

	void AudioSource::Reload()
	{
		VX_CORE_ASSERT(!m_Path.empty(), "Cannot load empty path!");
		LoadFromPathAndInitEngine(m_Path);
	}

	void AudioSource::SetProperties(const SoundProperties& soundProps)
    {
		SetDirection(soundProps.Direction);
		SetVelocity(soundProps.Velocity);
		SetCone(soundProps.Cone);
		SetMinDistance(soundProps.MinDistance);
		SetMaxDistance(soundProps.MaxDistance);
		SetPitch(soundProps.Pitch);
		SetDopplerFactor(soundProps.DopplerFactor);
		SetVolume(soundProps.Volume);
		SetPlayOnStart(soundProps.PlayOnStart);
		SetPlayOneShot(soundProps.PlayOneShot);
		SetSpacialized(soundProps.Spacialized);
		SetLoop(soundProps.Loop);
    }

    float AudioSource::GetAmountComplete()
	{
		return AudioEngine::GetSoundCursor(&m_Sound) / m_LengthInSeconds;
	}

	void AudioSource::Copy(SharedRef<AudioSource> dest, const SharedRef<AudioSource>& src)
	{
		const auto& props = src->GetProperties();
		const AudioSource::SoundProperties& srcProps = src->GetProperties();
		dest->SetProperties(srcProps);
	}

	void AudioSource::LoadFromPathAndInitEngine(const std::string& filepath)
	{
		VX_CORE_ASSERT(!filepath.empty(), "Cannot load empty file!");

		AudioEngine::InitEngine(&m_Engine);

		AudioEngine::InitSoundFromPath(
			&m_Engine,
			filepath,
			&m_Sound,
			&m_LengthInSeconds,
			m_Properties.Loop,
			m_Properties.Spacialized,
			m_Properties.Volume
		);

		m_Initialized = true;
	}

	SharedRef<AudioSource> AudioSource::Create(const std::string& filepath)
	{
		return CreateShared<AudioSource>(filepath);
	}

	SharedRef<AudioSource> AudioSource::Create()
	{
		return CreateShared<AudioSource>();
    }

}

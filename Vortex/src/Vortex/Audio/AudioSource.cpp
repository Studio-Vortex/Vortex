#include "vxpch.h"
#include "AudioSource.h"

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

	const AudioClip& AudioSource::GetAudioClip() const
	{
		return m_AudioClip;
	}

	void AudioSource::Play()
	{
		if (!m_IsLoaded)
		{
			Reload();
		}

		m_PlaybackDevice.Play();
	}

	void AudioSource::PlayOneShot()
	{
		if (!m_IsLoaded)
		{
			Reload();
		}

		VX_CORE_ASSERT(!m_Path.empty(), "Cannot play sound from empty filepath!");
		m_PlaybackDevice.PlayOneShot(m_Path);
	}

	void AudioSource::Pause()
	{
		if (!m_IsLoaded)
			return;

		m_PlaybackDevice.Pause();
	}

	void AudioSource::Restart()
	{
		if (!m_IsLoaded)
			return;

		m_PlaybackDevice.Restart();
	}

	void AudioSource::Stop()
	{
		if (!m_IsLoaded)
			return;

		Restart();
		m_PlaybackDevice.Stop();
		m_IsLoaded = false;
	}

	void AudioSource::Destroy()
	{
		if (!m_IsLoaded)
			return;
		
		m_PlaybackDevice.Shutdown();
	}

	bool AudioSource::IsPlaying()
	{
		return m_PlaybackDevice.IsPlaying();
	}

	void AudioSource::SetPosition(const Math::vec3& position)
	{
		m_PlaybackDevice.SetPosition(position);
		m_Properties.Position = position;
	}

	void AudioSource::SetDirection(const Math::vec3& direction)
	{
		m_PlaybackDevice.SetDirection(direction);
		m_Properties.Direction = direction;
	}

	void AudioSource::SetVelocity(const Math::vec3& velocity)
	{
		m_PlaybackDevice.SetVelocity(velocity);
		m_Properties.Velocity = velocity;
	}

	void AudioSource::SetCone(const SoundProperties::AudioCone& cone)
	{
		m_PlaybackDevice.SetCone(cone.InnerAngle, cone.OuterAngle, cone.OuterGain);
		m_Properties.Cone = cone;
	}

	void AudioSource::SetMinDistance(float minDistance)
	{
		m_PlaybackDevice.SetMinDistance(minDistance);
		m_Properties.MinDistance = minDistance;
	}

	void AudioSource::SetMaxDistance(float maxDistance)
	{
		m_PlaybackDevice.SetMaxDistance(maxDistance);
		m_Properties.MaxDistance = maxDistance;
	}

	void AudioSource::SetPitch(float pitch)
	{
		m_PlaybackDevice.SetPitch(pitch);
		m_Properties.Pitch = pitch;
	}

	void AudioSource::SetDopplerFactor(float dopplerFactor)
	{
		m_PlaybackDevice.SetDopplerFactor(dopplerFactor);
		m_Properties.DopplerFactor = dopplerFactor;
	}

	void AudioSource::SetVolume(float volume)
	{
		m_PlaybackDevice.SetVolume(volume);
		m_Properties.Volume = volume;
	}

	void AudioSource::SetSpacialized(bool spacialized)
	{
		m_PlaybackDevice.SetSpacialized(spacialized);
		m_Properties.Spacialized = spacialized;
	}

	void AudioSource::SetLoop(bool loop)
	{
		m_PlaybackDevice.SetLooping(loop);
		m_Properties.Loop = loop;
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
		VX_CORE_ASSERT(!m_Path.empty(), "Cannot load sound from empty filepath!");
		LoadFromPathAndInitEngine(m_Path);
	}

	void AudioSource::SetProperties(const SoundProperties& soundProps)
    {
		m_Properties = soundProps;
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
		return m_PlaybackDevice.GetSoundCursor() / m_AudioClip.Length;
	}

	void AudioSource::Copy(SharedReference<AudioSource>& dest, const SharedReference<AudioSource>& src)
	{
		const auto& props = src->GetProperties();
		const SoundProperties& srcProps = src->GetProperties();
		dest->SetProperties(srcProps);
	}

	void AudioSource::LoadFromPathAndInitEngine(const std::string& filepath)
	{
		VX_CORE_ASSERT(!filepath.empty(), "Cannot load empty file!");

		m_IsLoaded = m_PlaybackDevice.Init(filepath, &m_AudioClip.Length);

		if (!m_IsLoaded)
			return;

		size_t lastSlashPos = filepath.find_last_of("/\\");
		std::string filename = filepath.substr(lastSlashPos + 1);
		m_AudioClip.Name = filename;
	}

	SharedReference<AudioSource> AudioSource::Create(const std::string& filepath)
	{
		return SharedReference<AudioSource>::Create(filepath);
	}

	SharedReference<AudioSource> AudioSource::Create()
	{
		return SharedReference<AudioSource>::Create();
    }

}

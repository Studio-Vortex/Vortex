#include "vxpch.h"
#include "AudioSource.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	AudioSource::AudioSource(const std::string& filepath)
	{
		m_AudioClip.Filepath = filepath;
		Reload();
	}

	AudioSource::~AudioSource()
	{
		Destroy();
	}

	void AudioSource::LoadFromPathAndInitEngine(const std::string& filepath)
	{
		VX_CORE_ASSERT(!filepath.empty(), "Cannot load empty file!");

		m_IsLoaded = m_PlaybackDevice.Init(filepath, &m_AudioClip.Length);

		if (!m_IsLoaded)
			return;

		m_AudioClip.Name = FileSystem::RemoveFileExtension(filepath);
	}

	void AudioSource::Reload()
	{
		VX_CORE_ASSERT(!m_AudioClip.Filepath.empty(), "Cannot load sound from empty filepath!");
		LoadFromPathAndInitEngine(m_AudioClip.Filepath);
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

		VX_CORE_ASSERT(!m_AudioClip.Filepath.empty(), "Cannot play sound from empty filepath!");
		m_PlaybackDevice.PlayOneShot(m_AudioClip.Filepath);
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
		if (!m_IsLoaded)
			return false;

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

	void AudioSource::SetCone(const AudioCone& cone)
	{
		m_PlaybackDevice.SetCone(cone.InnerAngle, cone.OuterAngle, cone.OuterGain);
		m_Properties.Cone = cone;
	}

	void AudioSource::SetAttenuationModel(AttenuationModel attenuationModel)
	{
		m_PlaybackDevice.SetAttenuationModel(attenuationModel);
		m_Properties.AttenuationModel = attenuationModel;
	}

	void AudioSource::SetFalloff(float falloff)
	{
		m_PlaybackDevice.SetFalloff(falloff);
		m_Properties.Falloff = falloff;
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

	void AudioSource::SetLooping(bool loop)
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

	const std::string& AudioSource::GetPath() const
	{
		return m_AudioClip.Filepath;
	}

	void AudioSource::SetPath(const std::string& filepath)
	{
		m_AudioClip.Filepath = filepath;
	}

	const AudioClip& AudioSource::GetAudioClip() const
	{
		return m_AudioClip;
	}

	float AudioSource::GetAmountComplete()
	{
		return m_PlaybackDevice.GetSoundCursor() / m_AudioClip.Length;
	}

	PlaybackDevice& AudioSource::GetPlaybackDevice()
	{
		return m_PlaybackDevice;
	}

	const PlaybackDevice& AudioSource::GetPlaybackDevice() const
	{
		return m_PlaybackDevice;
	}

	const PlaybackDeviceProperties& AudioSource::GetProperties() const
	{
		return m_Properties;
	}

	PlaybackDeviceProperties& AudioSource::GetProperties()
	{
		return m_Properties;
	}

	void AudioSource::SetProperties(const PlaybackDeviceProperties& soundProps)
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
		SetLooping(soundProps.Loop);
    }

	void AudioSource::Copy(SharedReference<AudioSource>& dest, const SharedReference<AudioSource>& src)
	{
		const auto& props = src->GetProperties();
		const PlaybackDeviceProperties& srcProps = src->GetProperties();
		dest->SetProperties(srcProps);
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

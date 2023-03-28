#include "vxpch.h"
#include "AudioListener.h"

namespace Vortex {

	AudioListener::AudioListener(const ListenerProperties& props, PlaybackDevice& playbackDevice, uint32_t listenerIndex)
		: m_Properties(props), m_PlaybackDevice(playbackDevice), m_ListenerIndex(listenerIndex)
	{
		m_PlaybackDevice.AddDeviceListener(m_ListenerIndex);

		m_ListenerDevice.SetPlaybackDevice(m_PlaybackDevice);
		m_ListenerDevice.SetListenerIndex(m_ListenerIndex);

		m_ListenerDevice.SetWorldUp({ 0.0f, 1.0f, 0.0f });
	}

	AudioListener::~AudioListener()
	{
		m_PlaybackDevice.RemoveDeviceListener(m_ListenerDevice.GetListenerIndex());
	}

	void AudioListener::SetPosition(const Math::vec3& position)
	{
		m_ListenerDevice.SetPosition(position);
	}

	void AudioListener::SetDirection(const Math::vec3& direction)
	{
		m_ListenerDevice.SetDirection(direction);
	}

	void AudioListener::SetVelocity(const Math::vec3& velocity)
	{
		m_ListenerDevice.SetVelocity(velocity);
	}

	void AudioListener::SetCone(const AudioCone& cone)
	{
		m_ListenerDevice.SetCone(cone.InnerAngle, cone.OuterAngle, cone.OuterGain);
	}

	ListenerProperties& AudioListener::GetProperties()
	{
		return m_Properties;
	}

	const ListenerProperties& AudioListener::GetProperties() const
	{
		return m_Properties;
	}

	uint32_t AudioListener::GetListenerIndex() const
	{
		return m_ListenerIndex;
	}

	SharedReference<AudioListener> AudioListener::Create(const ListenerProperties& props, PlaybackDevice& device, uint32_t listenerIndex)
	{
		return SharedReference<AudioListener>::Create(props, device, listenerIndex);
	}

	SharedReference<AudioListener> AudioListener::Create()
	{
		return SharedReference<AudioListener>::Create();
	}

}

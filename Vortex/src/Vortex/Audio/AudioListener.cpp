#include "vxpch.h"
#include "AudioListener.h"

#include "Vortex/Audio/PlaybackDevice.h"

namespace Vortex {

	AudioListener::AudioListener(const ListenerDeviceProperties& props, PlaybackDevice& playbackDevice, uint32_t listenerIndex)
		: m_Properties(props), m_ListenerIndex(listenerIndex)
	{
		m_ListenerDevice.Init(playbackDevice, listenerIndex);
	}

	AudioListener::~AudioListener()
	{
		m_ListenerDevice.Shutdown();
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

	ListenerDeviceProperties& AudioListener::GetProperties()
	{
		return m_Properties;
	}

	const ListenerDeviceProperties& AudioListener::GetProperties() const
	{
		return m_Properties;
	}

	uint32_t AudioListener::GetListenerIndex() const
	{
		return m_ListenerIndex;
	}

	SharedReference<AudioListener> AudioListener::Create(const ListenerDeviceProperties& props, PlaybackDevice& device, uint32_t listenerIndex)
	{
		return SharedReference<AudioListener>::Create(props, device, listenerIndex);
	}

	SharedReference<AudioListener> AudioListener::Create()
	{
		return SharedReference<AudioListener>::Create();
	}

}

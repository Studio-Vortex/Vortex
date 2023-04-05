#include "vxpch.h"
#include "ListenerDevice.h"

#include "Vortex/Audio/AudioEngine.h"
#include "Vortex/Audio/PlaybackDevice.h"

namespace Vortex {

	void ListenerDevice::Init(PlaybackDevice& playbackDevice, uint8_t listenerIndex)
	{
		m_PlaybackDevice = &playbackDevice;
		m_ListenerIndex = listenerIndex;

		playbackDevice.AddDeviceListener(m_ListenerIndex);

		SetPlaybackDevice(playbackDevice);
		SetWorldUp({ 0.0f, 1.0f, 0.0f });
	}

	void ListenerDevice::Shutdown()
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		m_PlaybackDevice->RemoveDeviceListener(m_ListenerIndex);
	}

	void ListenerDevice::SetPlaybackDevice(PlaybackDevice& playbackDevice)
	{
		m_PlaybackDevice = &playbackDevice;
	}

	uint8_t ListenerDevice::GetListenerIndex() const
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		return m_ListenerIndex;
	}

	void ListenerDevice::SetListenerIndex(uint8_t listenerIndex)
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		m_ListenerIndex = listenerIndex;
	}

	bool ListenerDevice::IsListening() const
	{
		return m_PlaybackDevice != nullptr && m_ListenerIndex > -1;
	}

	void ListenerDevice::SetPosition(const Math::vec3& position)
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		AudioEngine::SetListenerPosition(m_PlaybackDevice->GetEngine(), m_ListenerIndex, position);
	}

	void ListenerDevice::SetDirection(const Math::vec3& direction)
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		AudioEngine::SetListenerDirection(m_PlaybackDevice->GetEngine(), m_ListenerIndex, direction);
	}

	void ListenerDevice::SetVelocity(const Math::vec3& velocity)
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		AudioEngine::SetListenerVeloctiy(m_PlaybackDevice->GetEngine(), m_ListenerIndex, velocity);
	}

	void ListenerDevice::SetCone(float innerAngleRad, float outerAngleRad, float outerGain)
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		AudioEngine::SetListenerCone(m_PlaybackDevice->GetEngine(), m_ListenerIndex, innerAngleRad, outerAngleRad, outerGain);
	}

	void ListenerDevice::SetWorldUp(const Math::vec3& up)
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		AudioEngine::SetListenerWorldUp(m_PlaybackDevice->GetEngine(), m_ListenerIndex, up);
	}

}

#include "vxpch.h"
#include "ListenerDevice.h"

#include "Vortex/Audio/AudioEngine.h"

namespace Vortex {
	
	void ListenerDevice::SetPlaybackDevice(PlaybackDevice& playbackDevice)
	{
		m_PlaybackDevice = &playbackDevice;
	}

	uint32_t ListenerDevice::GetListenerIndex() const
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		return m_ListenerIndex;
	}

	void ListenerDevice::SetListenerIndex(uint32_t listenerIndex)
	{
		VX_CORE_ASSERT(m_PlaybackDevice, "Device not initialized!");
		m_ListenerIndex = listenerIndex;
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

#pragma once

#include "Vortex/Core/Math/Math.h"
#include "Vortex/Audio/PlaybackDevice.h"

#include <cstdint>

namespace Vortex {

	class ListenerDevice
	{
	public:
		ListenerDevice() = default;
		~ListenerDevice() = default;

		void SetPlaybackDevice(PlaybackDevice& playbackDevice);

		uint32_t GetListenerIndex() const;
		void SetListenerIndex(uint32_t listenerIndex);

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(float innerAngleRad, float outerAngleRad, float outerGain);

		void SetWorldUp(const Math::vec3& up);

	private:
		uint32_t m_ListenerIndex = -1;
		PlaybackDevice* m_PlaybackDevice = nullptr;
	};

}

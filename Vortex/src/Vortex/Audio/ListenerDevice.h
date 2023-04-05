#pragma once

#include "Vortex/Core/Math/Math.h"

#include <cstdint>

namespace Vortex {

	class PlaybackDevice;

	class ListenerDevice
	{
	public:
		ListenerDevice() = default;
		~ListenerDevice() = default;

		void Init(PlaybackDevice& playbackDevice, uint8_t listenerIndex);
		void Shutdown();

		void SetPlaybackDevice(PlaybackDevice& playbackDevice);

		uint8_t GetListenerIndex() const;
		void SetListenerIndex(uint8_t listenerIndex);

		bool IsListening() const;

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(float innerAngleRad, float outerAngleRad, float outerGain);

		void SetWorldUp(const Math::vec3& up);

	private:
		int8_t m_ListenerIndex = -1;
		PlaybackDevice* m_PlaybackDevice = nullptr;
	};

}

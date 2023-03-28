#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Audio/ListenerProperties.h"
#include "Vortex/Audio/ListenerDevice.h"
#include "Vortex/Audio/PlaybackDevice.h"

namespace Vortex {

	class VORTEX_API AudioListener : public RefCounted
	{
	public:
		AudioListener() = default;
		AudioListener(const ListenerProperties& props, PlaybackDevice& playbackDevice, uint32_t listenerIndex);
		~AudioListener();

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(const AudioCone& cone);

		ListenerProperties& GetProperties();
		const ListenerProperties& GetProperties() const;

		uint32_t GetListenerIndex() const;

		static SharedReference<AudioListener> Create(const ListenerProperties& props, PlaybackDevice& device, uint32_t listenerIndex);
		static SharedReference<AudioListener> Create();

	private:
		uint32_t m_ListenerIndex = -1;
		ListenerProperties m_Properties;
		ListenerDevice m_ListenerDevice;
		PlaybackDevice m_PlaybackDevice;
	};

}

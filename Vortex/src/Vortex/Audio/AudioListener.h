#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Audio/ListenerDeviceProperties.h"
#include "Vortex/Audio/ListenerDevice.h"

namespace Vortex {

	class PlaybackDevice;

	class VORTEX_API AudioListener : public Asset
	{
	public:
		AudioListener() = default;
		AudioListener(const ListenerDeviceProperties& props, PlaybackDevice& playbackDevice, uint32_t listenerIndex);
		~AudioListener();

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(const AudioCone& cone);

		ListenerDeviceProperties& GetProperties();
		const ListenerDeviceProperties& GetProperties() const;

		uint32_t GetListenerIndex() const;

		ASSET_CLASS_TYPE(AudioListenerAsset)

		static SharedReference<AudioListener> Create(const ListenerDeviceProperties& props, PlaybackDevice& device, uint32_t listenerIndex);
		static SharedReference<AudioListener> Create();

	private:
		uint32_t m_ListenerIndex = -1;
		ListenerDeviceProperties m_Properties;
		ListenerDevice m_ListenerDevice;
	};

}

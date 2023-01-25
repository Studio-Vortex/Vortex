#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <miniaudio/miniaudio.h>

namespace Vortex {

	class VORTEX_API AudioListener : public RefCounted
	{
	public:
		struct VORTEX_API ListenerProperties
		{
			Math::vec3 Position = Math::vec3(0.0f);
			Math::vec3 Direction = Math::vec3(0.0f);
			Math::vec3 Veloctiy = Math::vec3(0.0f);

			struct VORTEX_API AudioCone
			{
				float InnerAngle = Math::Deg2Rad(10.0f);
				float OuterAngle = Math::Deg2Rad(45.0f);
				float OuterGain = 0.0f;
			} Cone;
		};

	public:
		AudioListener() = default;
		AudioListener(const ListenerProperties& props);
		~AudioListener();

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(const ListenerProperties::AudioCone& cone);

		inline const ListenerProperties& GetProperties() const { return m_Properties; }
		inline ListenerProperties& GetProperties() { return m_Properties; }

		static SharedRef<AudioListener> Create(const ListenerProperties& props);
		static SharedRef<AudioListener> Create();

	private:
		inline static uint32_t s_ListenerCount = 0;

	private:
		ListenerProperties m_Properties;
		uint32_t m_ListenerIndex = 0;
	};

}

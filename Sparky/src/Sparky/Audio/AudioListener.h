#pragma once

#include <miniaudio/miniaudio.h>

namespace Sparky {

	class AudioListener
	{
	public:
		struct ListenerProperties
		{
			Math::vec3 Position = Math::vec3(0.0f);
			Math::vec3 Direction = Math::vec3(0.0f);
			Math::vec3 Veloctiy = Math::vec3(0.0f);

			struct AudioCone
			{
				float InnerAngle = Math::Deg2Rad(10.0f);
				float OuterAngle = Math::Deg2Rad(45.0f);
				float OuterGain = 0.0f;
			} Cone;
		};

	public:
		AudioListener();
		~AudioListener();

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(const ListenerProperties::AudioCone& cone);

		inline const ListenerProperties& GetProperties() const { return m_Properties; }
		inline ListenerProperties& GetProperties() { return m_Properties; }

	private:
		inline static uint32_t s_ListenerCount = 0;

	private:
		uint32_t m_ListenerIndex;
		ListenerProperties m_Properties;
	};

}

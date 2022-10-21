#pragma once

#include <miniaudio/miniaudio.h>

namespace Sparky {

	class AudioSource
	{
	public:
		AudioSource(const std::string& filepath);
		~AudioSource() = default;

		void Play();
		void Stop();

		void Destroy();

		bool IsPlaying();

		void SetPosition(const Math::vec3& position);
		void SetVolume(float volume);

		void SetSpacialized(bool spacialized);
		void SetLoop(bool loop);

		inline const std::string& GetPath() const { return m_Path; }

		struct SoundProperties
		{
			Math::vec3 Position = Math::vec3(0.0f);
			float Volume = 1.0f;

			bool Loop = false;
			bool Spacialized = true;
		};

		inline const SoundProperties& GetProperties() const { return m_Properties; }
		inline SoundProperties& GetProperties() { return m_Properties; }

	private:
		bool m_Initialized = false;
		std::string m_Path;
		ma_sound m_Sound;
		SoundProperties m_Properties;
	};

}

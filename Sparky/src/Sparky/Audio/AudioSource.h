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

		void SetVolume(float volume);

		inline const std::string& GetPath() const { return m_Path; }

		struct SoundProperties
		{
			float Volume = 1.0f;
			bool Loop = false;
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

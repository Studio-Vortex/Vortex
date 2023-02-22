#pragma once

#include "Vortex/Core/Base.h"

#include <miniaudio/miniaudio.h>

namespace Vortex {

	class VORTEX_API AudioSource
	{
	public:
		struct VORTEX_API SoundProperties
		{
			Math::vec3 Position = Math::vec3(0.0f);
			Math::vec3 Direction = Math::vec3(0.0f);
			Math::vec3 Velocity = Math::vec3(0.0f);

			struct VORTEX_API AudioCone
			{
				float InnerAngle = Math::Deg2Rad(10.0f);
				float OuterAngle = Math::Deg2Rad(45.0f);
				float OuterGain = 0.0f;
			} Cone;

			float MinDistance = 1.0f;
			float MaxDistance = 10.0f;

			float Pitch = 1.0f;
			float DopplerFactor = 1.0f;
			float Volume = 1.0f;

			bool PlayOnStart = false;
			bool PlayOneShot = false;
			bool Loop = false;
			bool Spacialized = true;
		};

	public:
		AudioSource() = default;
		AudioSource(const std::string& filepath);
		~AudioSource();

		void Play();
		void Pause();
		void Restart();
		void Stop();

		void Destroy();

		bool IsPlaying();

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(const SoundProperties::AudioCone& cone);

		void SetMinDistance(float minDistance);
		void SetMaxDistance(float maxDistance);
		void SetPitch(float pitch);
		void SetDopplerFactor(float dopplerFactor);
		void SetVolume(float volume);

		void SetSpacialized(bool spacialized);
		void SetLoop(bool loop);
		void SetPlayOnStart(bool playOnStart);
		void SetPlayOneShot(bool playOneShot);

		inline const std::string& GetPath() const { return m_Path; }
		inline void SetPath(const std::string& filepath) { m_Path = filepath; }

		void Reload();

		inline const SoundProperties& GetProperties() const { return m_Properties; }
		inline SoundProperties& GetProperties() { return m_Properties; }
		void SetProperties(const SoundProperties& soundProps);

		inline static uint32_t AddAudioListener() { return ++s_ListenerCount; }
		inline static void RemoveAudioListener() { --s_ListenerCount; }

		float GetAmountComplete();

		static void Copy(SharedRef<AudioSource> dest, const SharedRef<AudioSource>& src);

		static SharedRef<AudioSource> Create(const std::string& filepath);
		static SharedRef<AudioSource> Create();

	private:
		void LoadFromPathAndInitEngine(const std::string& filepath);

	private:
		inline static uint32_t s_ListenerCount = 0;

	private:
		std::string m_Path;
		ma_engine m_Engine;
		ma_sound m_Sound;
		SoundProperties m_Properties;

		float m_LengthInSeconds = 0.0f;

		bool m_Initialized = false;
	};

}

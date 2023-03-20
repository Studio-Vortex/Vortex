#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <miniaudio/miniaudio.h>

namespace Vortex {

	struct VORTEX_API AudioClip
	{
		std::string Name = "";
		float Length = 0.0f;
	};

	class VORTEX_API AudioSource : public Asset
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
		~AudioSource() override;

		const AudioClip& GetAudioClip() const;

		void Play();
		void PlayOneShot();
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

		static void Copy(SharedReference<AudioSource>& dest, const SharedReference<AudioSource>& src);

		ASSET_CLASS_TYPE(AudioAsset)

		static SharedReference<AudioSource> Create(const std::string& filepath);
		static SharedReference<AudioSource> Create();

	private:
		void LoadFromPathAndInitEngine(const std::string& filepath);

	private:
		inline static uint32_t s_ListenerCount = 0;

	private:
		std::string m_Path;
		ma_engine m_Engine;
		ma_sound m_Sound;
		SoundProperties m_Properties;
		AudioClip m_AudioClip;

		bool m_IsLoaded = false;
	};

}

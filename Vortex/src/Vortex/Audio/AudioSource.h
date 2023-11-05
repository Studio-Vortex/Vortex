#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Audio/AttenuationModel.h"
#include "Vortex/Audio/PlaybackDeviceProperties.h"
#include "Vortex/Audio/PlaybackDevice.h"
#include "Vortex/Audio/AudioClip.h"

namespace Vortex {

	class VORTEX_API AudioSource : public Asset
	{
	public:
		AudioSource() = default;
		AudioSource(const std::string& filepath, bool isDefault = false);
		~AudioSource() override;

		void Reload();

		void Play();
		void PlayOneShot();
		void Pause();
		void Restart();
		void Stop();

		bool IsPlaying() const;
		bool IsPaused() const;

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(const AudioCone& cone);
		void SetMinGain(float minGain);
		void SetMaxGain(float maxGain);
		void SetAttenuationModel(AttenuationModel attenuationModel);
		void SetFalloff(float falloff);
		void SetMinDistance(float minDistance);
		void SetMaxDistance(float maxDistance);

		void SetPitch(float pitch);
		void SetDopplerFactor(float dopplerFactor);
		void SetVolume(float volume);
		void SetSpacialized(bool spacialized);
		void SetLooping(bool loop);

		void SetPlayOnStart(bool playOnStart);
		void SetPlayOneShot(bool playOneShot);

		const std::string& GetPath() const;
		void SetPath(const std::string& filepath);

		const AudioClip& GetAudioClip() const;
		float GetAmountComplete() const;

		PlaybackDevice& GetPlaybackDevice();
		const PlaybackDevice& GetPlaybackDevice() const;

		const PlaybackDeviceProperties& GetProperties() const;
		PlaybackDeviceProperties& GetProperties();
		void SetProperties(const PlaybackDeviceProperties& soundProps);

		static void Copy(SharedReference<AudioSource>& dest, const SharedReference<AudioSource>& src);

		ASSET_CLASS_TYPE(AudioAsset)

		static SharedReference<AudioSource> Create(const std::string& filepath, bool isDefault = false);
		static SharedReference<AudioSource> Create();

	private:
		PlaybackDevice m_PlaybackDevice;
		PlaybackDeviceProperties m_Properties;
		AudioClip m_AudioClip;

		bool m_IsLoaded = false;
		bool m_IsDefault = false;
	};

}

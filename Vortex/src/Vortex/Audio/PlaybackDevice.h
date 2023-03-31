#pragma once

#include "Vortex/Core/Math/Math.h"
#include "Vortex/Audio/AttenuationModel.h"

#include <miniaudio/miniaudio.h>

#include <string>

namespace Vortex {

	class PlaybackDevice
	{
	public:
		inline static uint8_t MaxDeviceListeners = MA_ENGINE_MAX_LISTENERS;

	public:
		PlaybackDevice();
		~PlaybackDevice();

		// Controls

		bool Load(const std::string& filepath, float* length);

		void Play();
		void PlayOneShot(const std::string& filepath);
		void Pause();
		void Restart();
		void Stop();

		// Properties

		bool IsPlaying() const;
		bool IsPaused() const;
		float GetSoundCursor() const;

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);

		void SetCone(float innerAngleRad, float outerAngleRad, float outerGain);
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
		void SetLooping(bool looping);

		uint8_t GetDeviceListenerCount() const;
		void AddDeviceListener(uint8_t listenerIndex);
		void RemoveDeviceListener(uint8_t listenerIndex);

		// Internal

		ma_engine* GetEngine();
		ma_sound* GetSound();

	private:
		mutable ma_engine m_Engine;
		mutable ma_sound m_Sound;

		bool m_IsPaused = false;
	};

}

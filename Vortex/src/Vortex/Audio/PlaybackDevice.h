#pragma once

#include "Vortex/Core/Math/Math.h"

#include "Vortex/Audio/SoundProperties.h"

#include <miniaudio/miniaudio.h>

#include <string>

namespace Vortex {

	class PlaybackDevice
	{
	public:
		PlaybackDevice() = default;
		~PlaybackDevice() = default;

		// Controls

		bool Init(const std::string& filepath, float* length);
		void Shutdown();

		void Play();
		void PlayOneShot(const std::string& filepath);
		void Pause();
		void Restart();
		void Stop();

		float GetSoundCursor();

		// Properties

		bool IsPlaying();

		void SetPosition(const Math::vec3& position);
		void SetDirection(const Math::vec3& direction);
		void SetVelocity(const Math::vec3& velocity);
		void SetCone(float innerAngleRad, float outerAngleRad, float outerGain);
		void SetMinDistance(float minDistance);
		void SetMaxDistance(float maxDistance);
		void SetPitch(float pitch);
		void SetDopplerFactor(float dopplerFactor);
		void SetVolume(float volume);
		void SetSpacialized(bool spacialized);
		void SetLooping(bool looping);

		// Internal

		ma_engine* GetEngine();
		ma_sound* GetSound();

	private:
		void PlaySound();
		void PlayOneShotFromFilepath(const std::string& filepath);
		void PauseSound();
		void RestartSound();
		void StopSound();
		void DestroySound();

		void StartEngine();
		void ShutdownEngine();

	private:
		ma_engine m_Engine;
		ma_sound m_Sound;
	};

}

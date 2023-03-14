#pragma once

#include "Vortex/Core/Math/Math.h"

#include <string>

struct ma_engine;
struct ma_sound;

namespace Vortex {

	class AudioEngine
	{
	public:
		static void InitEngine(ma_engine* engine);
		static void ShutdownEngine(ma_engine* engine);

		static void InitSoundFromPath(ma_engine* preInitializedEngine, const std::string& filepath, ma_sound* sound, float* length, bool loop, bool spacialized = true, float volume = 1.0f);
		static void DestroySound(ma_sound* sound);

		static void PlayFromSound(ma_sound* sound);
		static void PlayOneShot(ma_engine* engine, const char* filepath);
		static void PauseSound(ma_sound* sound);
		static void RestartSound(ma_sound* sound);
		static void StopSound(ma_sound* sound);

		static void SetPosition(ma_sound* sound, const Math::vec3& position);
		static void SetDirection(ma_sound* sound, const Math::vec3& direction);

		static void SetListenerPosition(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& position);
		static void SetListenerDirection(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& direction);

		static void SetVeloctiy(ma_sound* sound, const Math::vec3& veloctiy);

		static void SetCone(ma_sound* sound, float innerAngleRadians, float outerAngleRadians, float outerGain);

		static void SetMinDistance(ma_sound* sound, float minDistance);
		static void SetMaxDistance(ma_sound* sound, float maxDistance);
		static void SetPitch(ma_sound* sound, float pitch);
		static void SetDopplerFactor(ma_sound* sound, float dopplerFactor);
		static void SetVolume(ma_sound* sound, float volume);

		static void SetSpacialized(ma_sound* sound, bool spacialized);
		static void SetLoop(ma_sound* sound, bool loop);

		static float GetSoundCursor(ma_sound* sound);

		static bool IsPlaying(ma_sound* sound);

		static void StartEngine(ma_engine* engine);
		static void StopEngine(ma_engine* engine);
	};

}

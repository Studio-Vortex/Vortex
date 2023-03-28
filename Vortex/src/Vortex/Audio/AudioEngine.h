#pragma once

#include "Vortex/Core/Math/Math.h"

#include <string>

struct ma_engine;
struct ma_sound;

namespace Vortex {

	class AudioEngine
	{
	public:
		// Controls

		static void InitEngine(ma_engine* engine);
		static void ShutdownEngine(ma_engine* engine);

		static void InitSoundFromPath(ma_engine* initializedEngine, const std::string& filepath, ma_sound* sound, float* length);
		static void DestroySound(ma_sound* sound);

		// NOTE: Can't name this PlaySound because of Win32 api
		static void PlayFromSound(ma_sound* sound);
		static void PlayOneShot(ma_engine* engine, const char* filepath);
		static void PauseSound(ma_sound* sound);
		static void RestartSound(ma_sound* sound);
		static void StopSound(ma_sound* sound);

		// Sound Properties

		static float GetSoundCursor(ma_sound* sound);
		static bool IsSoundPlaying(ma_sound* sound);

		static void SetSoundPosition(ma_sound* sound, const Math::vec3& position);
		static void SetSoundDirection(ma_sound* sound, const Math::vec3& direction);
		static void SetSoundVeloctiy(ma_sound* sound, const Math::vec3& veloctiy);

		static void SetSoundCone(ma_sound* sound, float innerAngleRadians, float outerAngleRadians, float outerGain);
		static void SetSoundMinDistance(ma_sound* sound, float minDistance);
		static void SetSoundMaxDistance(ma_sound* sound, float maxDistance);

		static void SetSoundPitch(ma_sound* sound, float pitch);
		static void SetSoundDopplerFactor(ma_sound* sound, float dopplerFactor);
		static void SetSoundVolume(ma_sound* sound, float volume);
		static void SetSoundSpacialized(ma_sound* sound, bool spacialized);
		static void SetSoundLooping(ma_sound* sound, bool loop);

		// Engine Listeners

		static uint32_t GetDeviceListenerCount(ma_engine* engine);
		static void AddDeviceListener(ma_engine* engine, uint32_t listenerIndex);
		static void RemoveDeviceListener(ma_engine* engine, uint32_t listenerIndex);
		static bool IsListenerEnabled(ma_engine* engine, uint32_t listenerIndex);

		// Listener Properties

		static void SetListenerPosition(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& position);
		static void SetListenerDirection(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& direction);
		static void SetListenerVeloctiy(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& veloctiy);

		static void SetListenerCone(ma_engine* engine, uint32_t listenerIndex, float innerAngleRadians, float outerAngleRadians, float outerGain);
		static void SetListenerWorldUp(ma_engine* engine, uint32_t listenerIndex, const Math::vec3& up);

		// Internal

		static void StartEngine(ma_engine* engine);
		static void StopEngine(ma_engine* engine);
	};

}

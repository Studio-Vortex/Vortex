#pragma once

// Forward declaration
struct ma_sound;

namespace Sparky {

	class AudioEngine
	{
	public:
		AudioEngine() = delete;
		AudioEngine(const AudioEngine&) = delete;

		static void Init();
		static void Shutdown();

		static void InitSoundFromPath(const std::string& filepath, ma_sound* sound, bool loop, float volume);

		static void PlayFromSound(ma_sound* sound);
		static void StopSound(ma_sound* sound);

		static void SetVolume(ma_sound* sound, float volume);

		static bool IsPlaying(ma_sound* sound);
		
		static void StartEngine();
		static void StopEngine();
	};

}

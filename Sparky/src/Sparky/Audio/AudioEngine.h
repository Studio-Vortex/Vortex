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

		static void InitSoundFromPath(const std::string& filepath, ma_sound* sound, bool loop, bool spacialized = true, float volume = 1.0f, bool editorSound = false);
		static void DestroySound(ma_sound* sound);

		static void RemoveLoadedSound(ma_sound* sound);
		static void DestroyLoadedSounds();

		static void PlayFromSound(ma_sound* sound);
		static void StopSound(ma_sound* sound);

		static void SetPosition(ma_sound* sound, const Math::vec3& position);
		static void SetVolume(ma_sound* sound, float volume);
		static void SetSpacialized(ma_sound* sound, bool spacialized);
		static void SetLoop(ma_sound* sound, bool loop);

		static bool IsPlaying(ma_sound* sound);

		static void StartEngine();
		static void StopEngine();
	};

}

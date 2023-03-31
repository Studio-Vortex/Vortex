#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include <string>

namespace Vortex {

	class Scene;
	class Entity;
	class AudioContext;

	class VORTEX_API AudioSystem
	{
	public:
		enum class AudioAPI
		{
			None = 0, MiniAudio = 1,
		};

	public:
		static void Init();
		static void Shutdown();

		static void SubmitContextScene(Scene* context);
		static void RemoveContextScene(Scene* context);

		static void CreateAudioSource(Entity& entity, Scene* context, const std::string& filepath = std::string());
		static void DestroyAudioSource(Entity& entity, Scene* context);

		static void CreateAudioListener(Entity& entity, Scene* context);
		static void DestroyAudioListener(Entity& entity, Scene* context);

		static void StartAudioSourcesRuntime(Scene* context);
		static void PauseAudioSourcesRuntime(Scene* context);
		static void ResumeAudioSourcesRuntime(Scene* context);
		static void StopAudioSourcesRuntime(Scene* context);

		static AudioAPI GetAudioAPI();

		static SharedReference<AudioContext> GetAudioContext();
	};

}

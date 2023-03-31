#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	class Scene;
	class Entity;

	class VORTEX_API AudioSystem
	{
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
	};

}

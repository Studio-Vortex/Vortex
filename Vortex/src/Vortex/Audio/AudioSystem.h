#pragma once

#include "Vortex/System/IAssetSystem.h"

#include <string>

namespace Vortex {

	class AudioContext;

	class VORTEX_API AudioSystem : public IAssetSystem
	{
	public:
		enum class AudioAPI
		{
			None = 0, MiniAudio = 1,
		};

	public:
		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void SubmitContextScene(Scene* context) override;
		virtual void RemoveContextScene(Scene* context) override;

		virtual void CreateAsset(Entity& entity, Scene* context) override;
		virtual void CreateAsset(Entity& entity, Scene* context, const std::string& filepath);
		virtual void DestroyAsset(Entity& entity, Scene* context) override;

		virtual void CreateAudioListener(Entity& entity, Scene* context);
		virtual void DestroyAudioListener(Entity& entity, Scene* context);

		virtual void OnRuntimeStart(Scene* context) override;
		virtual void OnUpdateRuntime(Scene* context) override;
		virtual void OnRuntimeScenePaused(Scene* context) override;
		virtual void OnRuntimeSceneResumed(Scene* context) override;
		virtual void OnRuntimeStop(Scene* context) override;

		static AudioAPI GetAudioAPI();

		static SharedReference<AudioContext> GetAudioContext();

		ASSET_SYSTEM_TYPE(AudioAsset)
	};

}

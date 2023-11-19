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
		AudioSystem();
		virtual ~AudioSystem() override = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void SubmitContextScene(Scene* context) override;
		virtual void RemoveContextScene() override;

		virtual void CreateAsset(Entity& entity) override;
		virtual void CreateAsset(Entity& entity, const std::string& filepath);
		virtual void DestroyAsset(Entity& entity) override;

		virtual void CreateAudioListener(Entity& entity);
		virtual void DestroyAudioListener(Entity& entity);

		virtual void OnRuntimeStart() override;
		virtual void OnUpdateRuntime() override;
		virtual void OnRuntimeScenePaused() override;
		virtual void OnRuntimeSceneResumed() override;
		virtual void OnRuntimeStop() override;

		static AudioAPI GetAudioAPI();

		static SharedReference<AudioContext> GetAudioContext();

		virtual void OnGuiRender() override;

		ASSET_SYSTEM_TYPE(AudioAsset)
	};

}

#pragma once

#include "Vortex/System/IAssetSystem.h"

namespace Vortex {

	class VORTEX_API AudioSystem : public IAssetSystem
	{
	public:
		AudioSystem();
		virtual ~AudioSystem() override = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void OnContextSceneCreated(Scene* context) override;
		virtual void OnContextSceneDestroyed(Scene* context) override;

		virtual void CreateAsset(Actor& actor) override;
		virtual void DestroyAsset(Actor& actor) override;

		virtual void OnRuntimeStart(Scene* context) override;
		virtual void OnUpdateRuntime(Scene* context) override;
		virtual void OnRuntimeScenePaused(Scene* context) override;
		virtual void OnRuntimeSceneResumed(Scene* context) override;
		virtual void OnRuntimeStop(Scene* context) override;

		virtual void OnGuiRender() override;

		ASSET_SYSTEM_TYPE(AudioAsset)
	};

}

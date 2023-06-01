#pragma once

#include "Vortex/System/IAssetSystem.h"
#include "Vortex/Core/TimeStep.h"

namespace Vortex {

	class VORTEX_API ParticleSystem : public IAssetSystem
	{
	public:
		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void SubmitContextScene(Scene* context) override;
		virtual void RemoveContextScene(Scene* context) override;

		virtual void CreateAsset(Entity& entity, Scene* context) override;
		virtual void DestroyAsset(Entity& entity, Scene* context) override;

		virtual void OnRuntimeStart(Scene* context) override;
		virtual void OnUpdateRuntime(Scene* context) override {}
		void OnUpdateRuntime(Scene* context, TimeStep delta);
		virtual void OnRuntimeScenePaused(Scene* context) override;
		virtual void OnRuntimeSceneResumed(Scene* context) override;
		virtual void OnRuntimeStop(Scene* context) override;

		ASSET_SYSTEM_TYPE(ParticleAsset)
	};

}
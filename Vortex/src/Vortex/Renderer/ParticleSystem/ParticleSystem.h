#pragma once

#include "Vortex/System/IAssetSystem.h"
#include "Vortex/Core/TimeStep.h"

namespace Vortex {

	class VORTEX_API ParticleSystem : public IAssetSystem
	{
	public:
		ParticleSystem();
		virtual ~ParticleSystem() override = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void OnContextSceneCreated(Scene* context) override;
		virtual void OnContextSceneDestroyed(Scene* context) override;

		virtual void CreateAsset(Entity& entity) override;
		virtual void DestroyAsset(Entity& entity) override;

		virtual void OnRuntimeStart(Scene* context) override;
		virtual void OnUpdateRuntime(Scene* context) override {}
		void OnUpdateRuntime(Scene* context, TimeStep delta);
		virtual void OnRuntimeScenePaused(Scene* context) override;
		virtual void OnRuntimeSceneResumed(Scene* context) override;
		virtual void OnRuntimeStop(Scene* context) override;

		virtual void OnGuiRender() override;

		ASSET_SYSTEM_TYPE(ParticleAsset)
	};

}

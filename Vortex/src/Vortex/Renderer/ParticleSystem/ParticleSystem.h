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
		virtual void OnContextSceneDestroyed() override;

		virtual void CreateAsset(Entity& entity) override;
		virtual void DestroyAsset(Entity& entity) override;

		virtual void OnRuntimeStart() override;
		virtual void OnUpdateRuntime() override {}
		void OnUpdateRuntime(TimeStep delta);
		virtual void OnRuntimeScenePaused() override;
		virtual void OnRuntimeSceneResumed() override;
		virtual void OnRuntimeStop() override;

		virtual void OnGuiRender() override;

		ASSET_SYSTEM_TYPE(ParticleAsset)
	};

}

#pragma once

#include "Vortex/System/ISystem.h"

namespace Vortex {

	class VORTEX_API UISystem : public ISystem
	{
		UISystem();
		virtual ~UISystem() override = default;

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void OnContextSceneCreated(Scene* context) override;
		virtual void OnContextSceneDestroyed(Scene* context) override;

		virtual void OnRuntimeStart(Scene* context) override;
		virtual void OnUpdateRuntime(Scene* context) override;
		virtual void OnRuntimeScenePaused(Scene* context) override;
		virtual void OnRuntimeSceneResumed(Scene* context) override;
		virtual void OnRuntimeStop(Scene* context) override;
		
		virtual void OnGuiRender() override;

		SYSTEM_CLASS_TYPE(UI)
	};

}

#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Asset/AssetTypes.h"

namespace Vortex {

#define ASSET_SYSTEM_TYPE(type) static AssetType GetStaticType() { return AssetType::##type; }\
								virtual AssetType GetAssetType() override { return GetStaticType(); }\

	class Scene;
	class Entity;

	class VORTEX_API IAssetSystem : public RefCounted
	{
	public:
		virtual void Init() = 0;
		virtual void Shutdown() = 0;

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetAssetType() = 0;

		virtual void SubmitContextScene(Scene* context) = 0;
		virtual void RemoveContextScene(Scene* context) = 0;

		virtual void CreateAsset(Entity& entity, Scene* context) = 0;
		virtual void DestroyAsset(Entity& entity, Scene* context) = 0;

		virtual void OnRuntimeStart(Scene* context) = 0;
		virtual void OnUpdateRuntime(Scene* context) = 0;
		virtual void OnRuntimeScenePaused(Scene* context) = 0;
		virtual void OnRuntimeSceneResumed(Scene* context) = 0;
		virtual void OnRuntimeStop(Scene* context) = 0;
	};

}

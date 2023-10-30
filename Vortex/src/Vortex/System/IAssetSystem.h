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
		IAssetSystem(const std::string& name = "Asset System") { m_DebugName = name; }
		virtual ~IAssetSystem() = default;

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

		// Debug

		inline const std::string& GetDebugName() const { return m_DebugName; }

		virtual void OnGuiRender() = 0;

	private:
		std::string m_DebugName;
	};

}

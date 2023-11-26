#include "vxpch.h"
#include "SystemManager.h"

namespace Vortex {

	void SystemManager::OnContextSceneCreated(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			assetSystem->OnContextSceneCreated(context);
		}
	}

	void SystemManager::OnContextSceneDestroyed(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			assetSystem->OnContextSceneDestroyed(context);
		}
	}

	void SystemManager::OnRuntimeStart(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "Invalid asset type!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeStart(context);
		}
	}

	void SystemManager::OnRuntimeScenePaused(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "Invalid asset type!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeScenePaused(context);
		}
	}

	void SystemManager::OnRuntimeSceneResumed(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "Invalid asset type!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeSceneResumed(context);
		}
	}

	void SystemManager::OnRuntimeStop(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
    
		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "Invalid asset type!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeStop(context);
		}
	}

	void SystemManager::OnGuiRender()
	{
		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "Invalid asset type!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnGuiRender();
		}
	}

}

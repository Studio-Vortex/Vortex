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

		for (auto& [systemType, system] : s_Systems)
		{
			system->OnContextSceneCreated(context);
		}
	}

	void SystemManager::OnContextSceneDestroyed(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			assetSystem->OnContextSceneDestroyed(context);
		}

		for (auto& [systemType, system] : s_Systems)
		{
			system->OnContextSceneDestroyed(context);
		}
	}

	void SystemManager::OnRuntimeStart(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledAssetSystems.contains(assetType), "Invalid asset type!");
			if (s_EnabledAssetSystems.at(assetType) == 0)
				continue;

			assetSystem->OnRuntimeStart(context);
		}

		for (auto& [systemType, system] : s_Systems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(systemType), "Invalid system type!");
			if (s_EnabledSystems.at(systemType) == 0)
				continue;

			system->OnRuntimeStart(context);
		}
	}

	void SystemManager::OnRuntimeScenePaused(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledAssetSystems.contains(assetType), "Invalid asset type!");
			if (s_EnabledAssetSystems.at(assetType) == 0)
				continue;

			assetSystem->OnRuntimeScenePaused(context);
		}

		for (auto& [systemType, system] : s_Systems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(systemType), "Invalid system type!");
			if (s_EnabledSystems.at(systemType) == 0)
				continue;

			system->OnRuntimeScenePaused(context);
		}
	}

	void SystemManager::OnRuntimeSceneResumed(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledAssetSystems.contains(assetType), "Invalid asset type!");
			if (s_EnabledAssetSystems.at(assetType) == 0)
				continue;

			assetSystem->OnRuntimeSceneResumed(context);
		}

		for (auto& [systemType, system] : s_Systems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(systemType), "Invalid system type!");
			if (s_EnabledSystems.at(systemType) == 0)
				continue;

			system->OnRuntimeSceneResumed(context);
		}
	}

	void SystemManager::OnRuntimeStop(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");
    
		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledAssetSystems.contains(assetType), "Invalid asset type!");
			if (s_EnabledAssetSystems.at(assetType) == 0)
				continue;

			assetSystem->OnRuntimeStop(context);
		}

		for (auto& [systemType, system] : s_Systems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(systemType), "Invalid system type!");
			if (s_EnabledSystems.at(systemType) == 0)
				continue;

			system->OnRuntimeStop(context);
		}
	}

	void SystemManager::OnGuiRender()
	{
		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledAssetSystems.contains(assetType), "Invalid asset type!");
			if (s_EnabledAssetSystems.at(assetType) == 0)
				continue;

			assetSystem->OnGuiRender();
		}

		for (auto& [systemType, system] : s_Systems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(systemType), "Invalid asset type!");
			if (s_EnabledSystems.at(systemType) == 0)
				continue;

			system->OnGuiRender();
		}
	}

}

#include "vxpch.h"
#include "SystemManager.h"

namespace Vortex {

	static Scene* s_ContextScene = nullptr;

	void SystemManager::SubmitContextScene(Scene* context)
	{
		VX_CORE_ASSERT(context, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			assetSystem->SubmitContextScene(context);
		}

		s_ContextScene = context;
	}

	void SystemManager::RemoveContextScene()
	{
		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			assetSystem->RemoveContextScene();
		}

		s_ContextScene = nullptr;
	}

	void SystemManager::OnRuntimeStart()
	{
		VX_CORE_ASSERT(s_ContextScene, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "AssetType not found!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeStart();
		}
	}

	void SystemManager::OnRuntimeScenePaused()
	{
		VX_CORE_ASSERT(s_ContextScene, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "AssetType not found!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeScenePaused();
		}
	}

	void SystemManager::OnRuntimeSceneResumed()
	{
		VX_CORE_ASSERT(s_ContextScene, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "AssetType not found!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeSceneResumed();
		}
	}

	void SystemManager::OnRuntimeStop()
	{
		VX_CORE_ASSERT(s_ContextScene, "Invalid scene!");

		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "AssetType not found!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnRuntimeStop();
		}
	}

	void SystemManager::OnGuiRender()
	{
		for (auto& [assetType, assetSystem] : s_AssetSystems)
		{
			VX_CORE_ASSERT(s_EnabledSystems.contains(assetType), "AssetType not found!");
			if (!s_EnabledSystems.at(assetType))
				continue;

			assetSystem->OnGuiRender();
		}
	}

}

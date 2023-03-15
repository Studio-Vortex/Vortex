#include "vxpch.h"
#include "AssetManager.h"

namespace Vortex {

	bool AssetManager::IsHandleValid(AssetHandle handle)
	{
		return Project::GetAssetManager()->IsHandleValid(handle);
	}

	bool AssetManager::IsMemoryOnlyAsset(AssetHandle handle)
	{
		return Project::GetAssetManager()->IsMemoryOnlyAsset(handle);
	}

	bool AssetManager::ReloadData(AssetHandle handle)
	{
		return Project::GetAssetManager()->ReloadData(handle);
	}

	AssetType AssetManager::GetAssetType(AssetHandle handle)
	{
		return Project::GetAssetManager()->GetAssetType(handle);
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& AssetManager::GetLoadedAssets()
	{
		return Project::GetAssetManager()->GetLoadedAssets();
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& AssetManager::GetMemoryOnlyAssets()
	{
		return Project::GetAssetManager()->GetMemoryOnlyAssets();
	}

}

#include "vxpch.h"
#include "AssetManager.h"

namespace Vortex {

	bool AssetManager::IsHandleValid(AssetHandle handle)
	{
		return false;
	}

	bool AssetManager::IsMemoryOnlyAsset(AssetHandle handle)
	{
		return false;
	}

	bool AssetManager::ReloadData(AssetHandle handle)
	{
		return false;
	}

	AssetType AssetManager::GetAssetType(AssetHandle handle)
	{
		return AssetType::None;
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& AssetManager::GetLoadedAssets()
	{
		return std::unordered_map<AssetHandle, SharedReference<Asset>>();
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& AssetManager::GetMemoryOnlyAssets()
	{
		return std::unordered_map<AssetHandle, SharedReference<Asset>>();
	}

}

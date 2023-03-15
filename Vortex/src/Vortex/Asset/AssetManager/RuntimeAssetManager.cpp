#include "vxpch.h"
#include "RuntimeAssetManager.h"

namespace Vortex {

	RuntimeAssetManager::RuntimeAssetManager()
	{
	}

	RuntimeAssetManager::~RuntimeAssetManager()
	{
	}

	AssetType RuntimeAssetManager::GetAssetType(AssetHandle handle) const
	{
		return AssetType::None;
	}

	SharedReference<Asset> RuntimeAssetManager::GetAsset(AssetHandle handle)
	{
		return SharedReference<Asset>();
	}

	void RuntimeAssetManager::AddMemoryOnlyAsset(SharedReference<Asset> asset)
	{
	}

	bool RuntimeAssetManager::ReloadData(AssetHandle handle)
	{
		return false;
	}

	bool RuntimeAssetManager::IsHandleValid(AssetHandle handle)
	{
		return false;
	}

	bool RuntimeAssetManager::IsMemoryOnlyAsset(AssetHandle handle)
	{
		return false;
	}

	bool RuntimeAssetManager::IsAssetLoaded(AssetHandle handle)
	{
		return false;
	}

	std::unordered_set<AssetHandle> RuntimeAssetManager::GetAllAssetsWithType(AssetType type) const
	{
		return std::unordered_set<AssetHandle>();
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& RuntimeAssetManager::GetLoadedAssets() const
	{
		return std::unordered_map<AssetHandle, SharedReference<Asset>>();
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& RuntimeAssetManager::GetMemoryOnlyAssets() const
	{
		return std::unordered_map<AssetHandle, SharedReference<Asset>>();
	}

}

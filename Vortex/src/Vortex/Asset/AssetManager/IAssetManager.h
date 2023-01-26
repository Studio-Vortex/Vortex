#pragma once

#include "Vortex/Asset/Asset.h"
#include "Vortex/Asset/AssetTypes.h"

#include <unordered_map>
#include <unordered_set>

namespace Vortex {

	class IAssetManager : public RefCounted
	{
	public:
		IAssetManager() = default;
		virtual ~IAssetManager() = default;

		virtual AssetType GetAssetType(AssetHandle handle) const = 0;
		virtual SharedRef<Asset> GetAsset(AssetHandle handle) const = 0;
		virtual void AddMemoryOnlyAsset(SharedRef<Asset> asset) = 0;
		virtual bool ReloadData(AssetHandle handle) = 0;
		virtual bool IsHandleValid(AssetHandle handle) const = 0;
		virtual bool IsMemoryOnlyAsset(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;

		virtual std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) const = 0;
		virtual const std::unordered_map<AssetHandle, SharedRef<Asset>>& GetLoadedAssets() const = 0;
		virtual const std::unordered_map<AssetHandle, SharedRef<Asset>>& GetMemoryOnlyAssets() const = 0;
	};

}

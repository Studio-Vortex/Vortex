#pragma once

#include "Vortex/Asset/AssetManager/IAssetManager.h"

namespace Vortex {

	class VORTEX_API RuntimeAssetManager : public IAssetManager
	{
	public:
		RuntimeAssetManager();
		~RuntimeAssetManager() override;

		virtual AssetType GetAssetType(AssetHandle handle) const override;
		virtual SharedReference<Asset> GetAsset(AssetHandle handle) override;
		virtual void AddMemoryOnlyAsset(SharedReference<Asset> asset) override;
		virtual bool ReloadData(AssetHandle handle) override;
		virtual bool IsHandleValid(AssetHandle handle) override;
		virtual bool IsMemoryOnlyAsset(AssetHandle handle) override;
		virtual bool IsAssetLoaded(AssetHandle handle) override;

		virtual std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) const override;
		virtual const std::unordered_map<AssetHandle, SharedReference<Asset>>& GetLoadedAssets() const override;
		virtual const std::unordered_map<AssetHandle, SharedReference<Asset>>& GetMemoryOnlyAssets() const override;
	};

}

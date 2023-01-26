#pragma once

#include "Vortex/Asset/AssetManager/IAssetManager.h"

#include "Vortex/Asset/AssetImporter.h"
#include "Vortex/Asset/AssetRegistry.h"

namespace Vortex {

	class EditorAssetManager : public IAssetManager
	{
	public:
		EditorAssetManager();
		~EditorAssetManager() override;

		AssetType GetAssetType(AssetHandle handle) const override;
		SharedRef<Asset> GetAsset(AssetHandle handle) const override;
		void AddMemoryOnlyAsset(SharedRef<Asset> asset) override;
		bool ReloadData(AssetHandle handle) override;
		bool IsHandleValid(AssetHandle handle) const override;
		bool IsMemoryOnlyAsset(AssetHandle handle) const override;
		bool IsAssetLoaded(AssetHandle handle) const override;
		std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) const override;
		const std::unordered_map<AssetHandle, SharedRef<Asset>>& GetLoadedAssets() const override;
		const std::unordered_map<AssetHandle, SharedRef<Asset>>& GetMemoryOnlyAssets() const override;

		const AssetRegistry& GetAssetRegistry() const;

	private:
		void LoadAssetRegistry();
		void ProcessDirectory();
		void ReloadAssets();
		void WriteToRegistryFile();

	private:
		std::unordered_map<AssetHandle, SharedRef<Asset>> m_LoadedAssets;
		std::unordered_map<AssetHandle, SharedRef<Asset>> m_MemoryOnlyAssets;
		AssetRegistry m_AssetRegistry;
	};

}

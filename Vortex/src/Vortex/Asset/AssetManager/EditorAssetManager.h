#pragma once

#include "Vortex/Asset/AssetManager/IAssetManager.h"

#include "Vortex/Asset/AssetImporter.h"
#include "Vortex/Asset/AssetRegistry.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	class EditorAssetManager : public IAssetManager
	{
	public:
		EditorAssetManager();
		~EditorAssetManager() override;

		void OnSerialized();
		void OnDeserialized();

		AssetType GetAssetType(AssetHandle handle) const override;
		SharedReference<Asset> GetAsset(AssetHandle handle) override;
		void AddMemoryOnlyAsset(SharedReference<Asset> asset) override;
		bool ReloadData(AssetHandle handle) override;
		VX_FORCE_INLINE bool IsHandleValid(AssetHandle handle) override { return IsMemoryOnlyAsset(handle) || GetMetadata(handle).IsValid(); }
		VX_FORCE_INLINE bool IsMemoryOnlyAsset(AssetHandle handle) override { return m_MemoryOnlyAssets.find(handle) != m_MemoryOnlyAssets.end(); }
		VX_FORCE_INLINE bool IsAssetLoaded(AssetHandle handle) override { return m_LoadedAssets.find(handle) != m_LoadedAssets.end(); }
		std::unordered_set<AssetHandle> GetAllAssetsWithType(AssetType type) const override;
		const std::unordered_map<AssetHandle, SharedReference<Asset>>& GetLoadedAssets() const override;
		const std::unordered_map<AssetHandle, SharedReference<Asset>>& GetMemoryOnlyAssets() const override;

		const AssetRegistry& GetAssetRegistry() const;

		std::filesystem::path GetRelativePath(const std::filesystem::path& filepath);

		SharedReference<Asset> GetAssetFromFilepath(const std::filesystem::path& filepath);
		AssetHandle GetAssetHandleFromFilepath(const std::filesystem::path& filepath);
		AssetType GetAssetTypeFromExtension(const std::string& extension);
		AssetType GetAssetTypeFromFilepath(const std::filesystem::path& filepath);

		const AssetMetadata& GetMetadata(const std::filesystem::path& filepath);
		const AssetMetadata& GetMetadata(AssetHandle handle);
		const AssetMetadata& GetMetadata(SharedReference<Asset> asset);

		std::filesystem::path GetFileSystemPath(const AssetMetadata& metadata);

		AssetHandle ImportAsset(const std::filesystem::path& filepath);

		template <typename TAsset, typename... Args>
		SharedReference<TAsset> CreateNewAsset(const std::string& filename, Args&&... args)
		{
			static_assert(std::is_base_of<Asset, TAsset>::value, "CreateNewAsset only works for types derived from Asset");

			std::string directoryPath = FileSystem::GetParentDirectory(filename).string();

			AssetMetadata metadata;
			metadata.Handle = AssetHandle();
			if (directoryPath.empty() || directoryPath == ".")
				metadata.Filepath = filename;
			else
				metadata.Filepath = GetRelativePath(directoryPath + "/" + filename);
			metadata.IsDataLoaded = true;
			metadata.Type = TAsset::GetStaticType();

			m_AssetRegistry[metadata.Handle] = metadata;

			WriteToRegistryFile();

			SharedReference<TAsset> asset = SharedReference<TAsset>::Create(std::forward<Args>(args)...);
			asset->Handle = metadata.Handle;
			m_LoadedAssets[metadata.Handle] = asset;
			AssetImporter::Serialize(asset);

			return asset;
		}

	private:
		void LoadAssetRegistry();
		void ProcessDirectory(const std::filesystem::path& directory);
		void ReloadAssets();
		void WriteToRegistryFile();

		AssetMetadata& GetMetadataInternal(AssetHandle handle);

	private:
		std::unordered_map<AssetHandle, SharedReference<Asset>> m_LoadedAssets;
		std::unordered_map<AssetHandle, SharedReference<Asset>> m_MemoryOnlyAssets;

		AssetRegistry m_AssetRegistry;
	};

}

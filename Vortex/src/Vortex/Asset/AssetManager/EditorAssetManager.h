#pragma once

#include "Vortex/Asset/AssetManager/IAssetManager.h"

#include "Vortex/Editor/DefaultMesh.h"

#include "Vortex/Asset/AssetImporter.h"
#include "Vortex/Asset/AssetRegistry.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	class AssetRegistryPanel;
	class ContentBrowserPanel;

	class EditorAssetManager : public IAssetManager
	{
	public:
		EditorAssetManager();
		~EditorAssetManager() override;

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

		Fs::Path GetRelativePath(const Fs::Path& filepath);

		SharedReference<Asset> GetAssetFromFilepath(const Fs::Path& filepath);
		AssetHandle GetAssetHandleFromFilepath(const Fs::Path& filepath);
		AssetType GetAssetTypeFromExtension(const std::string& extension);
		AssetType GetAssetTypeFromFilepath(const Fs::Path& filepath);

		bool IsValidAssetExtension(const Fs::Path& extension);

		const AssetMetadata& GetMetadata(const Fs::Path& filepath);
		const AssetMetadata& GetMetadata(AssetHandle handle);
		const AssetMetadata& GetMetadata(SharedReference<Asset> asset);
		AssetMetadata& GetMutableMetadata(AssetHandle handle);

		Fs::Path GetFileSystemPath(const AssetMetadata& metadata);

		AssetHandle ImportAsset(const Fs::Path& filepath);

		bool RenameAsset(SharedReference<Asset>& asset, const Fs::Path& newFilepath);

		AssetHandle GetDefaultStaticMesh(DefaultMesh::StaticMeshType defaultMesh);
		bool IsDefaultStaticMesh(AssetHandle assetHandle);

		template <typename TAsset, typename... Args>
		VX_FORCE_INLINE SharedReference<TAsset> CreateNewAsset(const std::string& directory, const std::string& filename, Args&&... args)
		{
			static_assert(std::is_base_of<Asset, TAsset>::value, "CreateNewAsset only works for types derived from Asset");

			AssetMetadata metadata;
			metadata.Handle = AssetHandle();
			if (directory.empty() || directory == ".")
				metadata.Filepath = filename;
			else
				metadata.Filepath = GetRelativePath(directory + "/" + filename);
			metadata.IsDataLoaded = true;
			metadata.Type = TAsset::GetStaticType();

			m_AssetRegistry[metadata.Handle] = metadata;

			WriteToRegistryFile();

			SharedReference<TAsset> asset = SharedReference<TAsset>::Create(std::forward<Args>(args)...);
			asset->Handle = metadata.Handle;
			m_LoadedAssets[metadata.Handle] = asset;
			AssetImporter::Serialize(asset);

			VX_CONSOLE_LOG_INFO("New Asset Created: Handle: '{}', Path: '{}'", metadata.Handle, metadata.Filepath.string());

			return asset;
		}

		bool OnProjectSerialized();
		bool OnProjectDeserialized();

	private:
		void LoadAssetRegistry();
		void ProcessDirectory(const Fs::Path& directory);
		void ReloadAssets();
		void WriteToRegistryFile();

		AssetMetadata& GetMetadataInternal(AssetHandle handle);

	private:
		std::unordered_map<AssetHandle, SharedReference<Asset>> m_LoadedAssets;
		std::unordered_map<AssetHandle, SharedReference<Asset>> m_MemoryOnlyAssets;

		AssetRegistry m_AssetRegistry;

		// used only to prevent crashing when closing the editor
		Fs::Path m_ProjectAssetDirectory;
		Fs::Path m_ProjectAssetRegistryPath;

	private:
		friend AssetRegistryPanel;
		friend ContentBrowserPanel;
	};

}

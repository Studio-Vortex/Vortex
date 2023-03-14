#include "vxpch.h"
#include "EditorAssetManager.h"

namespace Vortex {

	EditorAssetManager::EditorAssetManager()
	{
		AssetImporter::Init();

		LoadAssetRegistry();
		ReloadAssets();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		WriteToRegistryFile();
		AssetImporter::Shutdown();
	}

	AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
	{
		if (m_AssetRegistry.Contains(handle))
		{
			return m_AssetRegistry.Get(handle).Type;
		}

		VX_CORE_ASSERT(false, "Unknown Asset Type!");
		return AssetType::None;
	}

	SharedReference<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
	{
		if (m_LoadedAssets.contains(handle))
		{
			return m_LoadedAssets.at(handle);
		}
		else if (m_MemoryOnlyAssets.contains(handle))
		{
			return m_MemoryOnlyAssets.at(handle);
		}

		return nullptr;
	}

	void EditorAssetManager::AddMemoryOnlyAsset(SharedReference<Asset> asset)
	{
		AssetMetadata metadata;
		metadata.Handle = asset->Handle;
		metadata.IsDataLoaded = true;
		metadata.IsMemoryOnly = true;
		metadata.Type = asset->GetAssetType();
		m_AssetRegistry[asset->Handle] = metadata;

		m_MemoryOnlyAssets[asset->Handle] = asset;
	}

	bool EditorAssetManager::ReloadData(AssetHandle handle)
	{
		return false;
	}

	bool EditorAssetManager::IsHandleValid(AssetHandle handle) const
	{
		return false;
	}

	bool EditorAssetManager::IsMemoryOnlyAsset(AssetHandle handle) const
	{
		return false;
	}

	bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
	{
		return false;
	}

	std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type) const
	{
		return std::unordered_set<AssetHandle>();
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& EditorAssetManager::GetLoadedAssets() const
	{
		return m_LoadedAssets;
	}

	const std::unordered_map<AssetHandle, SharedReference<Asset>>& EditorAssetManager::GetMemoryOnlyAssets() const
	{
		return m_MemoryOnlyAssets;
	}

	const AssetRegistry& EditorAssetManager::GetAssetRegistry() const
	{
		return m_AssetRegistry;
	}

	void EditorAssetManager::LoadAssetRegistry()
	{
	}

	void EditorAssetManager::ProcessDirectory()
	{
	}

	void EditorAssetManager::ReloadAssets()
	{
	}

	void EditorAssetManager::WriteToRegistryFile()
	{
	}

}

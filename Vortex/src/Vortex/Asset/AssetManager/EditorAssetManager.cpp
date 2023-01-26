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
		return AssetType();
	}

	SharedRef<Asset> EditorAssetManager::GetAsset(AssetHandle handle) const
	{
		return SharedRef<Asset>();
	}

	void EditorAssetManager::AddMemoryOnlyAsset(SharedRef<Asset> asset)
	{
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

	const std::unordered_map<AssetHandle, SharedRef<Asset>>& EditorAssetManager::GetLoadedAssets() const
	{
		return m_LoadedAssets;
	}

	const std::unordered_map<AssetHandle, SharedRef<Asset>>& EditorAssetManager::GetMemoryOnlyAssets() const
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

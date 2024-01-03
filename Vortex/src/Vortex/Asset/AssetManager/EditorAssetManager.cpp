#include "vxpch.h"
#include "EditorAssetManager.h"

#include "Vortex/Core/String.h"

#include "Vortex/Asset/AssetExtensions.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Scene.h"

#include "Vortex/Renderer/Material.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

#include "Vortex/Utils/YAML_SerializationUtils.h"

#include <yaml-cpp/yaml.h>

namespace Vortex {

	static AssetMetadata s_NullMetadata;

	EditorAssetManager::EditorAssetManager()
		: m_ProjectAssetDirectory(Project::GetAssetDirectory()), m_ProjectAssetRegistryPath(Project::GetAssetRegistryPath())
	{
		AssetImporter::Init();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		WriteToRegistryFile();
	}

	bool EditorAssetManager::OnProjectSerialized()
	{
		WriteToRegistryFile();

		return true;
	}

	bool EditorAssetManager::OnProjectDeserialized()
	{
		LoadAssetRegistry();
		ReloadAssets();

		// Should we load default meshes here?
		DefaultMesh::Init();

		return true;
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

	SharedReference<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
	{
		VX_PROFILE_FUNCTION();

		if (IsMemoryOnlyAsset(handle))
		{
			return m_MemoryOnlyAssets[handle];
		}

		AssetMetadata& metadata = GetMetadataInternal(handle);
		if (!metadata.IsValid())
		{
			return nullptr;
		}

		SharedReference<Asset> asset = nullptr;
		if (!metadata.IsDataLoaded)
		{
			metadata.IsDataLoaded = AssetImporter::TryLoadData(metadata, asset);

			if (!metadata.IsDataLoaded)
			{
				return nullptr;
			}

			m_LoadedAssets[handle] = asset;
		}
		else
		{
			asset = m_LoadedAssets[handle];
		}

		return asset;
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

	bool EditorAssetManager::ReloadData(AssetHandle assetHandle)
	{
		AssetMetadata& metadata = GetMetadataInternal(assetHandle);
		if (!metadata.IsValid())
		{
			VX_CORE_ERROR("Trying to reload invalid asset");
			return false;
		}

		SharedReference<Asset> asset = nullptr;
		metadata.IsDataLoaded = AssetImporter::TryLoadData(metadata, asset);
		if (metadata.IsDataLoaded)
		{
			m_LoadedAssets[assetHandle] = asset;
		}

		return metadata.IsDataLoaded;
	}

	std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type) const
	{
		std::unordered_set<AssetHandle> assets;

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.Type != type)
				continue;

			assets.insert(handle);
		}

		return assets;
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

	Fs::Path EditorAssetManager::GetRelativePath(const Fs::Path& filepath)
	{
		Fs::Path relativePath = filepath.lexically_normal();
		const Fs::Path assetDirectory = m_ProjectAssetDirectory;
		const std::string temp = filepath.string();

		if (temp.find(assetDirectory.string()) == std::string::npos)
		{
			return relativePath;
		}

		relativePath = FileSystem::Relative(filepath, assetDirectory);

		if (relativePath.empty())
		{
			relativePath = filepath.lexically_normal();
		}

		return relativePath;
	}

	SharedReference<Asset> EditorAssetManager::GetAssetFromFilepath(const Fs::Path& filepath)
	{
		const AssetMetadata& metadata = GetMetadata(filepath);

		if (AssetMetadata::Equal(metadata, s_NullMetadata))
		{
			return nullptr;
		}

		if (IsHandleValid(metadata.Handle))
		{
			return GetAsset(metadata.Handle);
		}

		return nullptr;
	}

	AssetHandle EditorAssetManager::GetAssetHandleFromFilepath(const Fs::Path& filepath)
	{
		const SharedReference<Asset> asset = GetAssetFromFilepath(filepath);

		if (asset == nullptr)
		{
			return 0;
		}

		if (IsHandleValid(asset->Handle))
		{
			return asset->Handle;
		}

		return 0;
	}

	AssetType EditorAssetManager::GetAssetTypeFromExtension(const std::string& extension)
	{
		std::string_view copy(extension.begin(), extension.end());
		const std::string ext = String::ToLowerCopy(copy);

		if (!IsValidAssetExtension(ext))
		{
			return AssetType::None;
		}

		return s_AssetExtensionMap.at(ext);
	}

	AssetType EditorAssetManager::GetAssetTypeFromFilepath(const Fs::Path& filepath)
	{
		const std::string extension = FileSystem::GetFileExtension(filepath);
		return GetAssetTypeFromExtension(extension);
	}

	bool EditorAssetManager::IsValidAssetExtension(const Fs::Path& extension)
	{
		return s_AssetExtensionMap.contains(extension.string());
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const Fs::Path& filepath)
	{
		const Fs::Path relativePath = GetRelativePath(filepath);

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.Filepath != relativePath)
				continue;

			return metadata;
		}

		return s_NullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle)
	{
		if (m_AssetRegistry.Contains(handle))
		{
			return m_AssetRegistry.Get(handle);
		}

		return s_NullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(SharedReference<Asset> asset)
	{
		return GetMetadata(asset->Handle);
	}

	AssetMetadata& EditorAssetManager::GetMutableMetadata(AssetHandle handle)
	{
		if (m_AssetRegistry.Contains(handle))
		{
			return m_AssetRegistry[handle];
		}

		return s_NullMetadata;
	}

	Fs::Path EditorAssetManager::GetFileSystemPath(const AssetMetadata& metadata)
	{
		return m_ProjectAssetDirectory / metadata.Filepath;
	}

	AssetHandle EditorAssetManager::ImportAsset(const Fs::Path& filepath)
	{
		const Fs::Path path = GetRelativePath(filepath);

		if (const AssetMetadata& metadata = GetMetadata(path); metadata.IsValid())
		{
			return metadata.Handle;
		}

		AssetType type = GetAssetTypeFromFilepath(filepath);
		if (type == AssetType::None)
		{
			return 0;
		}

		AssetMetadata metadata;
		metadata.Filepath = path;
		metadata.Handle = AssetHandle();
		metadata.Type = type;
		
		m_AssetRegistry[metadata.Handle] = metadata;

		return metadata.Handle;
	}

	bool EditorAssetManager::RenameAsset(SharedReference<Asset>& asset, const Fs::Path& newFilepath)
	{
		if (!asset || !IsHandleValid(asset->Handle))
		{
			VX_CORE_ASSERT(false, "Trying to rename invalid asset!");
			return false;
		}

		if (newFilepath.empty())
		{
			VX_CORE_ASSERT(false, "Trying to rename asset with empty filepath!")
			return false;
		}

		if (IsMemoryOnlyAsset(asset->Handle))
		{
			
		}
		else
		{
			AssetMetadata& metadata = GetMutableMetadata(asset->Handle);
			const Fs::Path oldMetadataPath = metadata.Filepath;
			std::string temp = oldMetadataPath.string();
			size_t lastSlashPos = temp.find_last_of("/\\");
			const Fs::Path directory = temp.substr(0, lastSlashPos + 1);
			temp = newFilepath.string();
			lastSlashPos = temp.find_last_of("/\\");
			const std::string filenameWithExtension = temp.substr(lastSlashPos + 1, temp.size());
			const std::string filename = FileSystem::RemoveFileExtension(filenameWithExtension);
			metadata.Filepath = Fs::Path(directory / filenameWithExtension);

			switch (metadata.Type)
			{
				case AssetType::MaterialAsset: asset.As<Material>()->SetName(filename); break;
				case AssetType::SceneAsset: asset.As<Scene>()->SetDebugName(filename); break;
				case AssetType::ParticleAsset: asset.As<ParticleEmitter>()->SetName(filename); break;
			}

			AssetImporter::Serialize(asset);
		}

		return true;
	}

	AssetHandle EditorAssetManager::GetDefaultStaticMesh(DefaultMesh::StaticMeshType defaultMesh)
	{
		return DefaultMesh::DefaultStaticMeshes[(size_t)defaultMesh];
	}

	bool EditorAssetManager::IsDefaultStaticMesh(AssetHandle assetHandle)
	{
		return DefaultMesh::IsDefaultStaticMesh(assetHandle);
	}

	void EditorAssetManager::LoadAssetRegistry()
	{
		VX_CONSOLE_LOG_INFO("[Asset Manager] Loading Asset Registry");

		if (!FileSystem::Exists(m_ProjectAssetRegistryPath))
		{
			return;
		}

		std::ifstream stream(m_ProjectAssetRegistryPath);
		if (!stream.is_open())
		{
			const std::string assetRegistryPath = m_ProjectAssetRegistryPath.string();
			VX_CONSOLE_LOG_ERROR("[Asset Manager] Failed to open Asset Registry File /'{}'", assetRegistryPath);
		}

		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());

		YAML::Node assetHandles = data["Assets"];
		if (!assetHandles)
		{
			VX_CONSOLE_LOG_ERROR("[Asset Manager] Asset Registry was corrupted!");
			return;
		}

		for (auto entry : assetHandles)
		{
			const std::string filepath = entry["Filepath"].as<std::string>();
			const AssetHandle handle = entry["Handle"].as<uint64_t>();
			AssetType type = Utils::AssetTypeFromString(entry["Type"].as<std::string>());

			if (type == AssetType::None)
				continue;

			if (type != GetAssetTypeFromFilepath(filepath))
			{
				VX_CONSOLE_LOG_ERROR("[Asset Manager] Mismatch between AssetType and extension type while reading asset registry entry!");
				type = GetAssetTypeFromFilepath(filepath);
			}

			AssetMetadata metadata;
			metadata.Filepath = filepath;
			metadata.Handle = handle;
			metadata.Type = type;

			if (!FileSystem::Exists(GetFileSystemPath(metadata)))
			{
				VX_CONSOLE_LOG_INFO("[Asset Manager] Missing Asset '{}' detected in registry file, trying to locate...", metadata.Filepath);

				std::string mostLikelyCandidate;
				uint32_t bestScore = 0;

				for (const auto& pathEntry : std::filesystem::recursive_directory_iterator(m_ProjectAssetDirectory))
				{
					const Fs::Path& path = pathEntry.path();

					if (path.filename() != metadata.Filepath.filename())
					{
						continue;
					}

					if (bestScore > 0)
					{
						VX_CONSOLE_LOG_WARN("[Asset Manager] Multiple candidates found...");
					}

					std::vector<std::string> candidateParts = String::SplitString(path.string(), "/\\");

					uint32_t score = 0;

					for (const auto& part : candidateParts)
					{
						if (filepath.find(part) != std::string::npos)
							score++;
					}

					VX_CONSOLE_LOG_WARN("[Asset Manager] '{}' has a score of {}, best score is {}", path.string(), score, bestScore);

					if (bestScore > 0 && score == bestScore)
					{
						// TODO promp the user at this point
					}

					if (score <= bestScore)
						continue;

					bestScore = score;
					mostLikelyCandidate = path.string();
				}

				if (mostLikelyCandidate.empty() && bestScore == 0)
				{
					VX_CONSOLE_LOG_ERROR("[Asset Manager] Failed to locate a potential match for '{}'", metadata.Filepath);
					continue;
				}

				std::replace(mostLikelyCandidate.begin(), mostLikelyCandidate.end(), '\\', '/');
				metadata.Filepath = FileSystem::Relative(mostLikelyCandidate, m_ProjectAssetDirectory);
				VX_CONSOLE_LOG_WARN("[Asset Manager] Found most likely match '{}'", metadata.Filepath);
			}

			if (metadata.Handle == 0)
			{
				VX_CONSOLE_LOG_WARN("[Asset Manager] AssetHandle for '{}' is 0, this shouldn't happen", metadata.Filepath);
				continue;
			}

			m_AssetRegistry[metadata.Handle] = metadata;
		}

		VX_CONSOLE_LOG_INFO("[Asset Manager] Loaded {} asset entries", m_AssetRegistry.Count());
	}

	void EditorAssetManager::ProcessDirectory(const Fs::Path& directory)
	{
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			const Fs::Path filepath = entry.path();

			if (entry.is_directory())
			{
				ProcessDirectory(filepath);
				continue;
			}
			
			const Fs::Path extension = FileSystem::GetFileExtension(filepath);
			if (!IsValidAssetExtension(extension))
			{
				continue;
			}

			ImportAsset(filepath);
		}
	}

	void EditorAssetManager::ReloadAssets()
	{
		ProcessDirectory(m_ProjectAssetDirectory);
		WriteToRegistryFile();
	}

	void EditorAssetManager::WriteToRegistryFile()
	{
		struct AssetRegistryEntry
		{
			std::string Filepath = "";
			AssetType Type = AssetType::None;
		};

		std::map<UUID, AssetRegistryEntry> sortedMap;

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (!metadata.IsValid() || !IsHandleValid(handle))
				continue;

			if (!FileSystem::Exists(GetFileSystemPath(metadata)))
				continue;

			std::string filepathToSerialize = metadata.Filepath.string();

			// WINDOWS ONLY
			std::replace(filepathToSerialize.begin(), filepathToSerialize.end(), '\\', '/');
			sortedMap[metadata.Handle] = AssetRegistryEntry{ filepathToSerialize, metadata.Type };
		}

		VX_CORE_INFO("[Asset Manager] serializing asset registry with {} entries", m_AssetRegistry.Count());

		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Assets" << YAML::BeginSeq;
		
		for (const auto& [handle, entry] : sortedMap)
		{
			out << YAML::BeginMap;

			VX_SERIALIZE_PROPERTY(Handle, handle, out);
			VX_SERIALIZE_PROPERTY(Filepath, entry.Filepath, out);
			VX_SERIALIZE_PROPERTY(Type, Utils::StringFromAssetType(entry.Type), out);
			
			out << YAML::EndMap;
		}

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(m_ProjectAssetRegistryPath);

		VX_CORE_ASSERT(fout.is_open(), "Failed to open asset registry file!");

		fout << out.c_str();
	}

	AssetMetadata& EditorAssetManager::GetMetadataInternal(AssetHandle handle)
	{
		if (m_AssetRegistry.Contains(handle))
		{
			return m_AssetRegistry[handle];
		}

		return s_NullMetadata;
	}

}

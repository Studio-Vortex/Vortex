#include "vxpch.h"
#include "EditorAssetManager.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Asset/AssetExtensions.h"
#include "Vortex/Utils/StringUtils.h"
#include "Vortex/Utils/YAML_SerializationUtils.h"

#include <yaml-cpp/yaml.h>

namespace Vortex {

	static AssetMetadata s_NullMetadata;

	EditorAssetManager::EditorAssetManager()
	{
		AssetImporter::Init();
	}

	EditorAssetManager::~EditorAssetManager()
	{
		WriteToRegistryFile();
	}

	bool EditorAssetManager::OnSerialized()
	{
		WriteToRegistryFile();

		return true;
	}

	bool EditorAssetManager::OnDeserialized()
	{
		LoadAssetRegistry();
		ReloadAssets();

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
			return m_MemoryOnlyAssets[handle];

		auto& metadata = GetMetadataInternal(handle);
		if (!metadata.IsValid())
			return nullptr;

		SharedReference<Asset> asset = nullptr;
		if (!metadata.IsDataLoaded)
		{
			metadata.IsDataLoaded = AssetImporter::TryLoadData(metadata, asset);

			if (!metadata.IsDataLoaded)
				return nullptr;

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
		auto& metadata = GetMetadataInternal(assetHandle);
		if (!metadata.IsValid())
		{
			VX_CORE_ERROR("Trying to reload invalid asset");
			return false;
		}

		SharedReference<Asset> asset;
		//metadata.IsDataLoaded = AssetImporter::TryLoadData(metadata, asset);
		if (metadata.IsDataLoaded)
			m_LoadedAssets[assetHandle] = asset;

		return metadata.IsDataLoaded;
	}

	std::unordered_set<AssetHandle> EditorAssetManager::GetAllAssetsWithType(AssetType type) const
	{
		std::unordered_set<AssetHandle> result;

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.Type == type)
				result.insert(handle);
		}

		return result;
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

	std::filesystem::path EditorAssetManager::GetRelativePath(const std::filesystem::path& filepath)
	{
		std::filesystem::path relativePath = filepath.lexically_normal();
		std::filesystem::path assetDirectory = Project::GetAssetDirectory();
		std::string temp = filepath.string();

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

	SharedReference<Asset> EditorAssetManager::GetAssetFromFilepath(const std::filesystem::path& filepath)
	{
		const AssetMetadata& metadata = GetMetadata(filepath);

		if (IsHandleValid(metadata.Handle))
			return GetAsset(metadata.Handle);

		return nullptr;
	}

	AssetHandle EditorAssetManager::GetAssetHandleFromFilepath(const std::filesystem::path& filepath)
	{
		SharedReference<Asset> asset = GetAssetFromFilepath(filepath);

		if (IsHandleValid(asset->Handle));
			return asset->Handle;

		return 0;
	}

	AssetType EditorAssetManager::GetAssetTypeFromExtension(const std::string& extension)
	{
		std::string_view copy(extension.data());
		std::string ext = String::ToLowerCopy(copy);

		if (!IsValidAssetExtension(ext))
			return AssetType::None;

		return s_AssetExtensionMap.at(ext);
	}

	AssetType EditorAssetManager::GetAssetTypeFromFilepath(const std::filesystem::path& filepath)
	{
		std::string extension = FileSystem::GetFileExtension(filepath);
		return GetAssetTypeFromExtension(extension);
	}

	bool EditorAssetManager::IsValidAssetExtension(const std::filesystem::path& extension)
	{
		return s_AssetExtensionMap.contains(extension.string());
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(const std::filesystem::path& filepath)
	{
		const auto relativePath = GetRelativePath(filepath);

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			if (metadata.Filepath == relativePath)
				return metadata;
		}

		return s_NullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle)
	{
		if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry.Get(handle);

		return s_NullMetadata;
	}

	const AssetMetadata& EditorAssetManager::GetMetadata(SharedReference<Asset> asset)
	{
		return GetMetadata(asset->Handle);
	}

	std::filesystem::path EditorAssetManager::GetFileSystemPath(const AssetMetadata& metadata)
	{
		return Project::GetAssetDirectory() / metadata.Filepath;
	}

	AssetHandle EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
	{
		std::filesystem::path path = GetRelativePath(filepath);

		if (const auto& metadata = GetMetadata(path); metadata.IsValid())
			return metadata.Handle;

		AssetType type = GetAssetTypeFromFilepath(filepath);
		if (type == AssetType::None)
			return 0;

		AssetMetadata metadata;
		metadata.Filepath = path;
		metadata.Handle = AssetHandle();
		metadata.Type = type;
		
		m_AssetRegistry[metadata.Handle] = metadata;

		return metadata.Handle;
	}

	void EditorAssetManager::LoadAssetRegistry()
	{
		VX_CORE_INFO("[Asset Manager] Loading Asset Registry");

		const auto& assetRegistryPath = Project::GetAssetRegistryPath();
		if (!FileSystem::Exists(assetRegistryPath))
			return;

		std::ifstream stream(assetRegistryPath);
		VX_CORE_ASSERT(stream.is_open(), "Failed to open Asset Registry File!");
		std::stringstream ss;
		ss << stream.rdbuf();

		YAML::Node data = YAML::Load(ss.str());

		auto assetHandles = data["Assets"];
		if (!assetHandles)
		{
			VX_CORE_ERROR("Asset Registry was corrupted!");
			VX_CORE_ASSERT(false, "");
			return;
		}

		for (auto entry : assetHandles)
		{
			std::string filepath = entry["Filepath"].as<std::string>();
			AssetHandle handle = entry["Handle"].as<uint64_t>();
			AssetType type = Asset::GetAssetTypeFromString(entry["Type"].as<std::string>());

			if (type == AssetType::None)
				continue;

			if (type != GetAssetTypeFromFilepath(filepath))
			{
				VX_CONSOLE_LOG_ERROR("[Asset Manager] Mismatch between stored AssetType and extension type when reading asset registry!");
				type = GetAssetTypeFromFilepath(filepath);
			}

			AssetMetadata metadata;
			metadata.Filepath = filepath;
			metadata.Handle = handle;
			metadata.Type = type;

			if (!FileSystem::Exists(GetFileSystemPath(metadata)))
			{
				VX_CORE_INFO("[Asset Manager] Missing Asset '{}' detected in registry file, trying to locate...", metadata.Filepath);

				std::string mostLikelyCandidate;
				uint32_t bestScore = 0;

				for (const auto& pathEntry : std::filesystem::recursive_directory_iterator(Project::GetAssetDirectory()))
				{
					const std::filesystem::path& path = pathEntry.path();

					if (path.filename() != metadata.Filepath.filename())
					{
						continue;
					}

					if (bestScore > 0)
					{
						VX_CORE_WARN("[Asset Manager] Multiple candidates found...");
					}

					std::vector<std::string> candidateParts = String::SplitString(path.string(), "/\\");

					uint32_t score = 0;

					for (const auto& part : candidateParts)
					{
						if (filepath.find(part) != std::string::npos)
							score++;
					}

					VX_CORE_WARN("'{}' has a score of {}, best score is {}", path.string(), score, bestScore);

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
					VX_CORE_ERROR("[Asset Manager] Failed to locate a potential match for '{}'", metadata.Filepath);
					continue;
				}

				std::replace(mostLikelyCandidate.begin(), mostLikelyCandidate.end(), '\\', '/');
				metadata.Filepath = FileSystem::Relative(mostLikelyCandidate, Project::GetAssetDirectory());
				VX_CORE_WARN("[Asset Manager] Found most likely match '{}'", metadata.Filepath);
			}

			if (metadata.Handle == 0)
			{
				VX_CORE_WARN("[Asset Manager] AssetHandle for '{}' is 0, this shouldn't happen", metadata.Filepath);
				continue;
			}

			m_AssetRegistry[metadata.Handle] = metadata;
		}

		VX_CORE_INFO("[Asset Manager] Loaded {} asset entries", m_AssetRegistry.Count());
	}

	void EditorAssetManager::ProcessDirectory(const std::filesystem::path& directory)
	{
		for (const auto& entry : std::filesystem::directory_iterator(directory))
		{
			std::filesystem::path path = entry.path();

			if (path.string().find(".vxr") != std::string::npos)
				continue;

			if (entry.is_directory())
			{
				ProcessDirectory(path);
			}
			else
			{
				ImportAsset(path);
			}
		}
	}

	void EditorAssetManager::ReloadAssets()
	{
		ProcessDirectory(Project::GetAssetDirectory());
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
			VX_SERIALIZE_PROPERTY(Type, Asset::GetAssetNameFromType(entry.Type), out);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		const std::string& assetRegistryPath = Project::GetAssetRegistryPath().string();
		std::ofstream fout(assetRegistryPath);

		VX_CORE_ASSERT(fout.is_open(), "Failed to open asset registry file!");

		fout << out.c_str();
	}

	AssetMetadata& EditorAssetManager::GetMetadataInternal(AssetHandle handle)
	{
		if (m_AssetRegistry.Contains(handle))
			return m_AssetRegistry[handle];

		return s_NullMetadata;
	}

}

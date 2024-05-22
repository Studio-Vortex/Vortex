#pragma once

#include <Vortex.h>

#include <vector>

namespace Vortex {

	struct Directory;

	struct AssetEntry
	{
		AssetMetadata Metadata;
		Directory* Location = nullptr;
	};

	struct Directory
	{
		Fs::Path Path = "";
		std::string Stem = "";
		Directory* Parent = nullptr;
		std::vector<Directory> Children;
		std::vector<AssetEntry> Files;
	};

	class ProjectAssetDirectory : public RefCounted
	{
	public:
		ProjectAssetDirectory() = default;
		ProjectAssetDirectory(Directory assetDirectory);
		~ProjectAssetDirectory();

		const Directory& GetAssetDir() const { return m_AssetDirectory; }

	private:
		Directory m_AssetDirectory;
	};

}

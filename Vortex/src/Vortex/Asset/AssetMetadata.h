#pragma once

#include "Vortex/Asset/Asset.h"

#include <filesystem>

namespace Vortex {

	struct AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type = AssetType::None;

		std::filesystem::path Filepath;

		bool IsDataLoaded = false;
		bool IsMemoryOnly = false;
		
		inline bool IsValid() { return Handle != 0 && !IsMemoryOnly; }
	};

}

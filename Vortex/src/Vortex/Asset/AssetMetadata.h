#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Asset/Asset.h"

#include <filesystem>

namespace Vortex {

	struct VORTEX_API AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type = AssetType::None;

		std::filesystem::path Filepath;

		bool IsDataLoaded = false;
		bool IsMemoryOnly = false;
		
		inline bool IsValid() const
		{
			return Type != AssetType::None && Handle != 0 && !IsMemoryOnly;
		}
	};

}

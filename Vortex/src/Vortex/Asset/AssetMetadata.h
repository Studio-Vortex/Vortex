#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Asset/Asset.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	struct VORTEX_API AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type = AssetType::None;

		Fs::Path Filepath;

		bool IsDataLoaded = false;
		bool IsMemoryOnly = false;
		
		bool IsValid() const;

		static bool Equal(const AssetMetadata& lhs, const AssetMetadata& rhs);
	};

}

#include "vxpch.h"
#include "AssetMetadata.h"

namespace Vortex {

	bool AssetMetadata::IsValid() const
	{
		return Type != AssetType::None && Handle != 0 && !IsMemoryOnly;
	}

	bool AssetMetadata::Equal(const AssetMetadata& lhs, const AssetMetadata& rhs)
	{
		const bool typesEqual = lhs.Type == rhs.Type;
		const bool filepathsEqual = lhs.Filepath == rhs.Filepath;
		const bool handlesEqual = lhs.Handle == rhs.Handle;
		const bool consistent = (lhs.IsMemoryOnly && rhs.IsMemoryOnly) || (!lhs.IsMemoryOnly && !rhs.IsMemoryOnly);

		return typesEqual && filepathsEqual && handlesEqual && consistent;
	}

}

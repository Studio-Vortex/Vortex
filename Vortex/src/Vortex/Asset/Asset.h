#pragma once

#include "Vortex/Core/ReferenceCounting/RefCounted.h"
#include "Vortex/Asset/AssetTypes.h"
#include "Vortex/Core/UUID.h"

namespace Vortex {

	using AssetHandle = UUID;

	class Asset : public RefCounted
	{
	public:
		AssetHandle Handle;
		virtual ~Asset() = default;

		bool operator==(const Asset& other) const
		{
			return Handle == other.Handle;
		}

		bool operator!=(const Asset& other) const
		{
			return !(*this == other);
		}

		bool IsValid() const
		{
			return Handle != 0;
		}

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetAssetType() const { return AssetType::None; }
	};

}

#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

#include "Vortex/Asset/AssetTypes.h"

namespace Vortex {

#define ASSET_CLASS_TYPE(type) static AssetType GetStaticType() { return AssetType::##type; }\
								virtual AssetType GetAssetType() const override { return GetStaticType(); }

	using VORTEX_API AssetHandle = UUID;

	class VORTEX_API Asset : public RefCounted
	{
	public:
		AssetHandle Handle;

	public:
		virtual ~Asset() = default;

		bool operator==(const Asset& other) const { return Handle == other.Handle; }
		bool operator!=(const Asset& other) const { return !(*this == other); }

		bool IsValid() const { return Handle != 0; }

		static AssetType GetStaticType() { return AssetType::None; }
		virtual AssetType GetAssetType() const { return AssetType::None; }
	};

}

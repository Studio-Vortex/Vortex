#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

#include "Vortex/Asset/AssetTypes.h"

namespace Vortex {

	using VORTEX_API AssetHandle = UUID;

	class VORTEX_API Asset : public RefCounted
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

		static AssetType GetAssetTypeFromString(const std::string& name)
		{
			if (s_AssetTypesMap.contains(name))
				return s_AssetTypesMap[name];

			VX_CORE_ASSERT(false, "Unknown Asset Name!");
			return AssetType::None;
		}

		static std::string GetAssetNameFromType(AssetType type)
		{
			for (const auto& [name, assetType] : s_AssetTypesMap)
			{
				if (assetType == type)
					return name;
			}

			VX_CORE_ASSERT(false, "Unknown Asset Type!");
			return "None";
		}
	};

}

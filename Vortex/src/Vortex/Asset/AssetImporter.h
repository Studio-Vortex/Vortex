#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/AssetSerializer.h"

#include <unordered_map>

namespace Vortex {

	class AssetImporter
	{
	public:
		static void Init();

		static void Serialize(const AssetMetadata& metadata, const SharedReference<Asset>& asset);
		static void Serialize(const SharedReference<Asset>& asset);
		static bool TryLoadData(const AssetMetadata& metadata, SharedReference<Asset> asset);

	private:
		inline static std::unordered_map<AssetType, UniqueRef<AssetSerializer>> s_Serializers;
	};

}

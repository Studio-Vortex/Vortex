#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Asset/AssetSerializer.h"
#include "Vortex/Scene/Scene.h"

#include <unordered_map>

namespace Vortex {

	class AssetImporter
	{
	public:
		static void Init();
		static void Shutdown();

		static void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset);
		static void Serialize(const SharedRef<Asset>& asset);
		static bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset);

	private:
		inline static std::unordered_map<AssetType, UniqueRef<AssetSerializer>> s_Serializers;
	};

}

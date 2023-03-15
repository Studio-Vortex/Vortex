#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Asset/AssetSerializer.h"

#include <string>

namespace Vortex {

	struct PrefabComponent;

	class PrefabSerializer : public AssetSerializer
	{
	public:

		PrefabSerializer(const SharedRef<Prefab>& prefab);

		bool Serialize(const std::filesystem::path& path);
		bool Deserialize(const std::filesystem::path& path, PrefabComponent& prefabComponent);

		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;

	private:
		SharedRef<Prefab> m_Prefab = nullptr;
	};

}

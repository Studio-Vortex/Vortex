#pragma once

#include "Vortex/Core/Base.h"

#include <cstdint>
#include <string>

namespace Vortex {

	enum class VORTEX_API AssetType : uint16_t
	{
		None = 0,
		MeshAsset,
		FontAsset,
		AudioAsset,
		SceneAsset,
		PrefabAsset,
		ScriptAsset,
		TextureAsset,
		ParticleAsset,
		MaterialAsset,
		AnimatorAsset,
		AnimationAsset,
		StaticMeshAsset,
		EnvironmentAsset,
		AudioListenerAsset,
		PhysicsMaterialAsset,
	};

#define ASSET_CLASS_TYPE(type) static AssetType GetStaticType() { return AssetType::##type; }\
								virtual AssetType GetAssetType() const override { return GetStaticType(); }

	namespace Utils {

		AssetType AssetTypeFromString(const std::string& type);
		std::string StringFromAssetType(AssetType type);

	}

}

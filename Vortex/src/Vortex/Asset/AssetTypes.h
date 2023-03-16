#pragma once

#include "Vortex/Core/Base.h"

#include <unordered_map>
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
		MaterialAsset,
		AnimatorAsset,
		AnimationAsset,
		StaticMeshAsset,
		EnvironmentAsset,
		PhysicsMaterialAsset,
	};

#define ASSET_CLASS_NAME(type) #type, AssetType::##type

	static std::unordered_map<std::string, AssetType> s_AssetTypesMap =
	{
		{ ASSET_CLASS_NAME(MeshAsset) },
		{ ASSET_CLASS_NAME(FontAsset) },
		{ ASSET_CLASS_NAME(AudioAsset) },
		{ ASSET_CLASS_NAME(SceneAsset) },
		{ ASSET_CLASS_NAME(PrefabAsset) },
		{ ASSET_CLASS_NAME(ScriptAsset) },
		{ ASSET_CLASS_NAME(TextureAsset) },
		{ ASSET_CLASS_NAME(MaterialAsset) },
		{ ASSET_CLASS_NAME(AnimatorAsset) },
		{ ASSET_CLASS_NAME(AnimationAsset) },
		{ ASSET_CLASS_NAME(StaticMeshAsset) },
		{ ASSET_CLASS_NAME(EnvironmentAsset) },
		{ ASSET_CLASS_NAME(PhysicsMaterialAsset) },
	};

#define ASSET_CLASS_TYPE(type) static AssetType GetStaticType() { return AssetType::##type; }\
								virtual AssetType GetAssetType() const override { return GetStaticType(); }

}

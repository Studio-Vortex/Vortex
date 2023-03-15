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

	static std::unordered_map<std::string, AssetType> s_AssetTypes =
	{
		{ "MeshAsset", AssetType::MeshAsset },
		{ "FontAsset", AssetType::FontAsset },
		{ "AudioAsset", AssetType::AudioAsset },
		{ "SceneAsset", AssetType::SceneAsset },
		{ "PrefabAsset", AssetType::PrefabAsset },
		{ "ScriptAsset", AssetType::ScriptAsset },
		{ "TextureAsset", AssetType::TextureAsset },
		{ "MaterialAsset", AssetType::MaterialAsset },
		{ "AnimatorAsset", AssetType::AnimatorAsset },
		{ "AnimationAsset", AssetType::AnimationAsset },
		{ "StaticMeshAsset", AssetType::StaticMeshAsset },
		{ "EnvironmentAsset", AssetType::EnvironmentAsset },
		{ "PhysicsMaterialAsset", AssetType::PhysicsMaterialAsset },
	};

#define ASSET_CLASS_TYPE(type) static AssetType GetStaticType() { return AssetType::##type; }\
								virtual AssetType GetAssetType() const override { return GetStaticType(); }

}

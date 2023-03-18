#include "vxpch.h"
#include "AssetTypes.h"

namespace Vortex {

	namespace Utils {

		AssetType AssetTypeFromString(const std::string& type)
		{
			if (type == "MeshAsset")            return AssetType::MeshAsset;
			if (type == "FontAsset")            return AssetType::FontAsset;
			if (type == "AudioAsset")           return AssetType::AudioAsset;
			if (type == "SceneAsset")           return AssetType::SceneAsset;
			if (type == "PrefabAsset")          return AssetType::PrefabAsset;
			if (type == "ScriptAsset")          return AssetType::ScriptAsset;
			if (type == "TextureAsset")         return AssetType::TextureAsset;
			if (type == "MaterialAsset")        return AssetType::MaterialAsset;
			if (type == "AnimatorAsset")        return AssetType::AnimatorAsset;
			if (type == "AnimationAsset")       return AssetType::AnimationAsset;
			if (type == "StaticMeshAsset")      return AssetType::StaticMeshAsset;
			if (type == "EnvironmentAsset")     return AssetType::EnvironmentAsset;
			if (type == "PhysicsMaterialAsset") return AssetType::PhysicsMaterialAsset;

			VX_CORE_ASSERT(false, "Unknown Asset Type!");
			return AssetType::None;
		}

		std::string StringFromAssetType(AssetType type)
		{
			switch (type)
			{
				case AssetType::MeshAsset:            return "MeshAsset";
				case AssetType::FontAsset:            return "FontAsset";
				case AssetType::AudioAsset:           return "AudioAsset";
				case AssetType::SceneAsset:           return "SceneAsset";
				case AssetType::PrefabAsset:          return "PrefabAsset";
				case AssetType::ScriptAsset:          return "ScriptAsset";
				case AssetType::TextureAsset:         return "TextureAsset";
				case AssetType::MaterialAsset:        return "MaterialAsset";
				case AssetType::AnimatorAsset:        return "AnimatorAsset";
				case AssetType::AnimationAsset:       return "AnimationAsset";
				case AssetType::StaticMeshAsset:      return "StaticMeshAsset";
				case AssetType::EnvironmentAsset:     return "EnvironmentAsset";
				case AssetType::PhysicsMaterialAsset: return "PhysicsMaterialAsset";
			}

			VX_CORE_ASSERT(false, "Unknown Asset Type!");
			return "None";
		}

	}

}

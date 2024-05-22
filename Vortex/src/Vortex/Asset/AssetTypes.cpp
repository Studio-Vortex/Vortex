#include "vxpch.h"
#include "AssetTypes.h"

#include "Vortex/Core/String.h"

namespace Vortex {

	namespace Utils {

		AssetType AssetTypeFromString(const std::string& type)
		{
			if (String::FastCompare(type, "MeshAsset"))            return AssetType::MeshAsset;
			if (String::FastCompare(type, "FontAsset"))            return AssetType::FontAsset;
			if (String::FastCompare(type, "AudioAsset"))           return AssetType::AudioAsset;
			if (String::FastCompare(type, "SceneAsset"))           return AssetType::SceneAsset;
			if (String::FastCompare(type, "PrefabAsset"))          return AssetType::PrefabAsset;
			if (String::FastCompare(type, "ScriptAsset"))          return AssetType::ScriptAsset;
			if (String::FastCompare(type, "TextureAsset"))         return AssetType::TextureAsset;
			if (String::FastCompare(type, "ParticleAsset"))        return AssetType::ParticleAsset;
			if (String::FastCompare(type, "MaterialAsset"))        return AssetType::MaterialAsset;
			if (String::FastCompare(type, "AnimatorAsset"))        return AssetType::AnimatorAsset;
			if (String::FastCompare(type, "AnimationAsset"))       return AssetType::AnimationAsset;
			if (String::FastCompare(type, "StaticMeshAsset"))      return AssetType::StaticMeshAsset;
			if (String::FastCompare(type, "EnvironmentAsset"))     return AssetType::EnvironmentAsset;
			if (String::FastCompare(type, "AudioListenerAsset"))   return AssetType::AudioListenerAsset;
			if (String::FastCompare(type, "PhysicsMaterialAsset")) return AssetType::PhysicsMaterialAsset;

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
				case AssetType::ParticleAsset:        return "ParticleAsset";
				case AssetType::MaterialAsset:        return "MaterialAsset";
				case AssetType::AnimatorAsset:        return "AnimatorAsset";
				case AssetType::AnimationAsset:       return "AnimationAsset";
				case AssetType::StaticMeshAsset:      return "StaticMeshAsset";
				case AssetType::EnvironmentAsset:     return "EnvironmentAsset";
				case AssetType::AudioListenerAsset:   return "AudioListenerAsset";
				case AssetType::PhysicsMaterialAsset: return "PhysicsMaterialAsset";
			}

			VX_CORE_ASSERT(false, "Unknown Asset Type!");
			return "None";
		}

	}

}

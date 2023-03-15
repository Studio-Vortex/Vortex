#pragma once

#include "Vortex/Core/Base.h"

#include <cstdint>
#include <string>

namespace Vortex {

	enum class AssetType : uint16_t
	{
		None = 0,
		Mesh,
		StaticMesh,
		Font,
		Audio,
		Scene,
		Prefab,
		Script,
		Texture,
		Material,
		Animator,
		Animation,
		SubTexture,
		Environment,
		PhysicsMaterial,
	};

	namespace Utils {

		static const std::string& AssetTypeToString(AssetType type)
		{
			switch (type)
			{
				case AssetType::None:            return "None";
				case AssetType::Mesh:            return "Mesh";
				case AssetType::StaticMesh:      return "StaticMesh";
				case AssetType::Font:            return "Font";
				case AssetType::Audio:           return "Audio";
				case AssetType::Scene:           return "Scene";
				case AssetType::Prefab:          return "Prefab";
				case AssetType::Script:          return "Script";
				case AssetType::Texture:         return "Texture";
				case AssetType::Material:        return "Material";
				case AssetType::Animator:        return "Animator";
				case AssetType::Animation:       return "Animation";
				case AssetType::SubTexture:      return "SubTexture";
				case AssetType::Environment:     return "Environment";
				case AssetType::PhysicsMaterial: return "PhysicsMaterial";
			}

			VX_CORE_ASSERT(false, "Unknown Asset Type!");
			return "";
		}

		static AssetType AssetTypeFromString(const std::string& type)
		{
			if (type == "None")            return AssetType::None;
			if (type == "Mesh")            return AssetType::Mesh;
			if (type == "StaticMesh")      return AssetType::StaticMesh;
			if (type == "Font")            return AssetType::Font;
			if (type == "Audio")           return AssetType::Audio;
			if (type == "Scene")           return AssetType::Scene;
			if (type == "Prefab")          return AssetType::Prefab;
			if (type == "Script")          return AssetType::Script;
			if (type == "Texture")         return AssetType::Texture;
			if (type == "Material")        return AssetType::Material;
			if (type == "Animator")        return AssetType::Animator;
			if (type == "Animation")       return AssetType::Animation;
			if (type == "SubTexture")      return AssetType::SubTexture;
			if (type == "Environment")     return AssetType::Environment;
			if (type == "PhysicsMaterial") return AssetType::PhysicsMaterial;

			VX_CORE_ASSERT(false, "Unknown Asset Type!");
			return AssetType::None;
		}

	}

}

#pragma once

#include "Vortex/Asset/AssetTypes.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		{ ".vanimation", AssetType::Animation },
		{ ".vanimator", AssetType::Animator },

		{ ".wav", AssetType::Audio },
		{ ".mp3", AssetType::Audio },

		{ ".hdr", AssetType::Environment },
		
		{ ".ttf", AssetType::Font },
		{ ".TTF", AssetType::Font },
		{ ".otc", AssetType::Font },

		{ ".vmaterial", AssetType::Material },
		{ ".vpm", AssetType::PhysicsMaterial },

		{ ".fbx", AssetType::Mesh },
		{ ".dae", AssetType::Mesh },
		{ ".gltf", AssetType::Mesh },
		{ ".glb", AssetType::Mesh },
		{ ".obj", AssetType::Mesh },

		{ ".fbx", AssetType::StaticMesh },
		{ ".dae", AssetType::StaticMesh },
		{ ".gltf", AssetType::StaticMesh },
		{ ".glb", AssetType::StaticMesh },
		{ ".obj", AssetType::StaticMesh },

		{ ".vprefab", AssetType::Prefab },
		{ ".vortex", AssetType::Scene },

		{ ".cs", AssetType::Script },
		
		{ ".png", AssetType::SubTexture },
		{ ".jpg", AssetType::SubTexture },
		{ ".jpeg", AssetType::SubTexture },
		{ ".tga", AssetType::SubTexture },
		{ ".psd", AssetType::SubTexture },

		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".tga", AssetType::Texture },
		{ ".psd", AssetType::Texture },
	};

}

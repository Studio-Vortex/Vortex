#pragma once

#include "Vortex/Asset/AssetTypes.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		{ ".vanimation", AssetType::AnimationAsset },
		{ ".vanimator", AssetType::AnimatorAsset },

		{ ".wav", AssetType::AudioAsset },
		{ ".mp3", AssetType::AudioAsset },

		{ ".hdr", AssetType::EnvironmentAsset },
		{ ".HDR", AssetType::EnvironmentAsset },
		
		{ ".ttf", AssetType::FontAsset },
		{ ".TTF", AssetType::FontAsset },
		{ ".otc", AssetType::FontAsset },
		{ ".OTC", AssetType::FontAsset },

		{ ".vmaterial", AssetType::MaterialAsset },
		{ ".vpm", AssetType::PhysicsMaterialAsset },

		{ ".fbx", AssetType::MeshAsset },
		{ ".FBX", AssetType::MeshAsset },
		{ ".dae", AssetType::MeshAsset },
		{ ".DAE", AssetType::MeshAsset },
		{ ".gltf", AssetType::MeshAsset },
		{ ".GLTF", AssetType::MeshAsset },
		{ ".glb", AssetType::MeshAsset },
		{ ".GLB", AssetType::MeshAsset },
		{ ".obj", AssetType::MeshAsset },
		{ ".OBJ", AssetType::MeshAsset },

		{ ".fbx", AssetType::StaticMeshAsset },
		{ ".FBX", AssetType::StaticMeshAsset },
		{ ".dae", AssetType::StaticMeshAsset },
		{ ".DAE", AssetType::StaticMeshAsset },
		{ ".gltf", AssetType::StaticMeshAsset },
		{ ".GLTF", AssetType::StaticMeshAsset },
		{ ".glb", AssetType::StaticMeshAsset },
		{ ".GLB", AssetType::StaticMeshAsset },
		{ ".obj", AssetType::StaticMeshAsset },
		{ ".OBJ", AssetType::StaticMeshAsset },

		{ ".vprefab", AssetType::PrefabAsset },
		{ ".vortex", AssetType::SceneAsset },

		{ ".cs", AssetType::ScriptAsset },

		{ ".png", AssetType::TextureAsset },
		{ ".jpg", AssetType::TextureAsset },
		{ ".jpeg", AssetType::TextureAsset },
		{ ".tga", AssetType::TextureAsset },
		{ ".psd", AssetType::TextureAsset },
	};

}

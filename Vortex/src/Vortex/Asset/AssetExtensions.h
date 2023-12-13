#pragma once

#include "Vortex/Asset/AssetTypes.h"

#include <unordered_map>
#include <string>

namespace Vortex {

	std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		{ ".vanimation", AssetType::AnimationAsset },
		{ ".vanimator", AssetType::AnimatorAsset },

		{ ".vsound", AssetType::AudioAsset },
		{ ".wav", AssetType::AudioAsset },
		{ ".mp3", AssetType::AudioAsset },

		{ ".hdr", AssetType::EnvironmentAsset },
		{ ".HDR", AssetType::EnvironmentAsset },
		
		{ ".vfa", AssetType::FontAsset },
		{ ".ttf", AssetType::FontAsset },
		{ ".TTF", AssetType::FontAsset },
		{ ".otc", AssetType::FontAsset },
		{ ".OTC", AssetType::FontAsset },
		{ ".otf", AssetType::FontAsset },
		{ ".OTF", AssetType::FontAsset },

		{ ".vmaterial", AssetType::MaterialAsset },
		{ ".vpm", AssetType::PhysicsMaterialAsset },

		// TODO once we have skinned meshes come back to this with a specific file format
		// if we try to do it this way right now the asset system will get confused because
		// we have multiple extensions with two different AssetTypes

		/*{ ".fbx", AssetType::MeshAsset },
		{ ".FBX", AssetType::MeshAsset },
		{ ".dae", AssetType::MeshAsset },
		{ ".DAE", AssetType::MeshAsset },
		{ ".gltf", AssetType::MeshAsset },
		{ ".GLTF", AssetType::MeshAsset },
		{ ".glb", AssetType::MeshAsset },
		{ ".GLB", AssetType::MeshAsset },
		{ ".obj", AssetType::MeshAsset },
		{ ".OBJ", AssetType::MeshAsset },*/

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

		{ ".vortex", AssetType::SceneAsset },
		{ ".vprefab", AssetType::PrefabAsset },

		{ ".cs", AssetType::ScriptAsset },

		{ ".vparticle", AssetType::ParticleAsset },

		{ ".png", AssetType::TextureAsset },
		{ ".jpg", AssetType::TextureAsset },
		{ ".jpeg", AssetType::TextureAsset },
		{ ".tga", AssetType::TextureAsset },
		{ ".psd", AssetType::TextureAsset },
	};

}

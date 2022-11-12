#include "sppch.h"
#include "AssetRegistry.h"

#include "Sparky/Renderer/Texture.h"
#include "Sparky/Audio/AudioSource.h"
#include "Sparky/Renderer/Material.h"
#include "Sparky/Renderer/Model.h"

namespace Sparky {

	template <typename Texture2D, typename... Args>
	SharedRef<Texture2D> AssetRegistry::LoadAsset(const std::string& assetName, Args&&... args)
	{
		SharedRef<Texture2D> asset = Texture2D::Create(std::forward<Args>(args)...);
		s_Textures.push_back(std::make_pair(assetName, asset));
		return asset;
	}

	void AssetRegistry::ClearAssets()
	{
		for (auto& texture : s_Textures)
			texture.second.reset();

		s_Textures.clear();

		for (auto& audioSource : s_AudioSources)
			audioSource.second.reset();

		s_AudioSources.clear();

		for (auto& material : s_Materials)
			material.second.reset();

		s_Materials.clear();

		for (auto& model : s_Models)
			model.second.reset();

		s_Models.clear();
	}

}

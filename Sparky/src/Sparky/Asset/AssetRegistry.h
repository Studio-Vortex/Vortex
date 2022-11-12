#pragma once

#include "Sparky/Core/Base.h"

#include <vector>
#include <string>

namespace Sparky {

	class Texture2D;
	class AudioSource;
	class Material;
	class Model;

	class SPARKY_API AssetRegistry
	{
	public:
		template <typename TAsset, typename... Args>
		inline static SharedRef<TAsset> LoadAsset(const std::string& assetName, Args&&... args);

		template <typename TAsset>
		inline static SharedRef<TAsset> GetAsset(const std::string& assetName);

		inline static void ClearAssets();

	private:
		using TextureAsset  = std::pair<std::string, SharedRef<Texture2D>>;
		using AudioAsset    = std::pair<std::string, SharedRef<AudioSource>>;
		using MaterialAsset = std::pair<std::string, SharedRef<Material>>;
		using ModelAsset    = std::pair<std::string, SharedRef<Model>>;

	private:
		inline static std::vector<TextureAsset> s_Textures;
		inline static std::vector<AudioAsset> s_AudioSources;
		inline static std::vector<MaterialAsset> s_Materials;
		inline static std::vector<ModelAsset> s_Models;
	};

}

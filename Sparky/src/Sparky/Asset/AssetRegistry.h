#pragma once

#include "Sparky/Core/Base.h"

#include <unordered_map>
#include <string>

namespace Sparky {

	class Texture2D;
	class AudioSource;
	class Material;
	class Model;

	enum class AssetType
	{
		Texture = 0, AudioSource, Material, Model,
	};

	class SPARKY_API AssetRegistry
	{
	public:
		template <typename TAsset, typename ... Args>
		inline static SharedRef<TAsset> LoadAsset(const std::string& assetName, Args&& ... args);

		template <typename TAsset>
		inline static SharedRef<TAsset> GetAsset(const std::string& assetName);

		inline static void ClearAssets();

	private:
		inline static std::unordered_map<std::string, SharedRef<Texture2D>> s_Textures;
		inline static std::unordered_map<std::string, SharedRef<AudioSource>> s_AudioSources;
		inline static std::unordered_map<std::string, SharedRef<Material>> s_Materials;
		inline static std::unordered_map<std::string, SharedRef<Model>> s_Models;
	};

	template <typename TAsset, typename ... Args>
	inline SharedRef<TAsset> AssetRegistry::LoadAsset(const std::string& assetName, Args&& ... args)
	{
		SharedRef<TAsset> asset = TAsset::Create(std::forward<Args>);
	}

}

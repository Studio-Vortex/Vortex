#pragma once

#include "Sparky/Core/Base.h"

#include "Sparky/Renderer/Texture.h"
#include "Sparky/Audio/AudioSource.h"
#include "Sparky/Renderer/Material.h"
#include "Sparky/Renderer/Model.h"

#include <unordered_map>
#include <string>

namespace Sparky {

	class SPARKY_API AssetRegistry
	{
	public:
		static void Init();
		static void Shutdown();

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
		SharedRef<TAsset> asset = TAsset::Create(std::forward<Args>(args)...);

		if (std::is_same<TAsset, Texture2D>())
		{
			auto it = s_Textures.find(assetName);
			if (it != s_Textures.end())
			{
				s_Textures.insert(assetName, asset);
			}
			else
			{
				return it->second;
			}
		}
		else if (std::is_same<TAsset, AudioSource>())
		{
			auto it = s_AudioSources.find(assetName);
			if (it != s_AudioSources.end())
			{
				s_AudioSources.insert(assetName, asset);
			}
			else
			{
				return it->second;
			}
		}
		else if (std::is_same<TAsset, Material>())
		{
			auto it = s_Materials.find(assetName);
			if (it != s_Materials.end())
			{
				s_Materials.insert(assetName, asset);
			}
			else
			{
				return it->second;
			}
		}
		else if (std::is_same<TAsset, Model>())
		{
			auto it = s_Models.find(assetName);
			if (it != s_Models.end())
			{
				s_Models.insert(assetName, asset);
			}
			else
			{
				return it->second;
			}
		}

		SP_CORE_ASSERT(false, "Unknown Asset Type!");
		return nullptr;
	}

}

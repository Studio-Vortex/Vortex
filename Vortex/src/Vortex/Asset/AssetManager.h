#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Project/Project.h"
#include "Vortex/Utils/FileSystem.h"
#include "Vortex/Asset/Asset.h"

#include <unordered_map>
#include <unordered_set>

namespace Vortex {

	class VORTEX_API AssetManager
	{
	public:
		static bool IsHandleValid(AssetHandle handle);
		static bool IsMemoryOnlyAsset(AssetHandle handle);
		static bool ReloadData(AssetHandle handle);
		static AssetType GetAssetType(AssetHandle handle);

		static const std::unordered_map<AssetHandle, SharedReference<Asset>>& GetLoadedAssets();
		static const std::unordered_map<AssetHandle, SharedReference<Asset>>& GetMemoryOnlyAssets();

		template <typename TAsset>
		VX_FORCE_INLINE static SharedReference<TAsset> GetAsset(AssetHandle handle)
		{
			static_assert(std::is_base_of<Asset, TAsset>::value, "GetAsset only works with types derived from Asset!");

			SharedReference<Asset> asset = Project::GetAssetManager()->GetAsset(handle);

			return asset.As<TAsset>();
		}

		template <typename TAsset>
		VX_FORCE_INLINE static std::unordered_set<AssetHandle> GetAllAssetsWithType()
		{
			static_assert(std::is_base_of<Asset, TAsset>::value, "GetAllAssetsWithType only works with types derived from Asset!");

			return Project::GetAssetManager()->GetAllAssetsWithType(TAsset::GetStaticType());
		}

		template <typename TAsset, typename... TArgs>
		VX_FORCE_INLINE static AssetHandle CreateMemoryOnlyAsset(TArgs&&... args)
		{
			static_assert(std::is_base_of<Asset, TAsset>::value, "CreateMemoryOnlyAsset only works with types derived from Asset!");

			SharedReference<TAsset> asset = SharedReference<TAsset>::Create(std::forward<TArgs>(args)...);
			asset->Handle = AssetHandle();

			Project::GetAssetManager()->AddMemoryOnlyAsset(asset);

			return asset->Handle;
		}

		template <typename TAsset, typename... TArgs>
		VX_FORCE_INLINE static AssetHandle CreateMemoryOnlyAssetWithHandle(AssetHandle handle, TArgs&&... args)
		{
			static_assert(std::is_base_of<Asset, TAsset>::value, "CreateMemoryOnlyAssetWithHandle only works with types derived from Asset!");

			SharedReference<TAsset> asset = SharedReference<TAsset>::Create(std::forward<TArgs>(args)...);
			asset->Handle = handle;

			Project::GetAssetManager()->AddMemoryOnlyAsset(asset);

			return asset->Handle;
		}

		template <typename TAsset>
		VX_FORCE_INLINE static AssetHandle AddMemoryOnlyAsset(SharedReference<TAsset>& asset)
		{
			static_assert(std::is_base_of<Asset, TAsset>::value, "AddMemoryOnlyAsset only works with types derived from Asset!");

			asset->Handle = AssetHandle();

			Project::GetAssetManager()->AddMemoryOnlyAsset(asset);

			return asset->Handle;
		}
	};

}

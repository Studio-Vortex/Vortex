#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Project/Project.h"
#include "Vortex/Utils/FileSystem.h"
#include "Vortex/Utils/StringUtils.h"
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

		static const std::unordered_map<AssetHandle, SharedRef<Asset>>& GetLoadedAssets();
		static const std::unordered_map<AssetHandle, SharedRef<Asset>>& GetMemoryOnlyAssets();

		template <typename TAsset>
		VX_FORCE_INLINE static SharedRef<TAsset> GetAsset(AssetHandle handle)
		{
			return SharedRef<TAsset>();
		}

		template <typename TAsset>
		VX_FORCE_INLINE static std::unordered_set<AssetHandle> GetAllAssetsWithType()
		{
			return std::unordered_set<AssetHandle>();
		}

		template <typename TAsset, typename... TArgs>
		VX_FORCE_INLINE static AssetHandle CreateMemoryOnlyAsset(TArgs&&... args)
		{
			return AssetHandle();
		}

		template <typename TAsset, typename... TArgs>
		VX_FORCE_INLINE static AssetHandle CreateMemoryOnlyAssetWithHandle(AssetHandle handle, TArgs&&... args)
		{
			return AssetHandle();
		}

		template <typename TAsset>
		VX_FORCE_INLINE static AssetHandle AddMemoryOnlyAsset(SharedRef<TAsset> asset)
		{
			return AssetHandle();
		}
	};

}

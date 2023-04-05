#pragma once

#include "Vortex/Asset/AssetMetadata.h"

#include <unordered_map>

namespace Vortex {

	class AssetRegistry
	{
	public:
		AssetMetadata& operator[](const AssetHandle handle);
		AssetMetadata& Get(const AssetHandle handle);
		const AssetMetadata& Get(const AssetHandle handle) const;

		size_t Count() const;
		bool Contains(AssetHandle handle) const;
		std::unordered_map<AssetHandle, AssetMetadata>::iterator Find(AssetHandle handle);
		const std::unordered_map<AssetHandle, AssetMetadata>::const_iterator Find(AssetHandle handle) const;
		size_t Remove(AssetHandle handle);
		void Clear();

		inline std::unordered_map<AssetHandle, AssetMetadata>::iterator begin() { return m_LoadedAssets.begin(); }
		inline std::unordered_map<AssetHandle, AssetMetadata>::iterator end() { return m_LoadedAssets.end(); }
		inline std::unordered_map<AssetHandle, AssetMetadata>::const_iterator begin() const { return m_LoadedAssets.cbegin(); }
		inline std::unordered_map<AssetHandle, AssetMetadata>::const_iterator end() const { return m_LoadedAssets.cend(); }

	private:
		std::unordered_map<AssetHandle, AssetMetadata> m_LoadedAssets;
	};

}

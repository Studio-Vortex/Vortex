#include "vxpch.h"
#include "AssetRegistry.h"

namespace Vortex {
	
	AssetMetadata& AssetRegistry::operator[](const AssetHandle handle)
	{
		return m_LoadedAssets[handle];
	}

	AssetMetadata& AssetRegistry::Get(const AssetHandle handle)
	{
		return m_LoadedAssets.find(handle)->second;
	}

	const AssetMetadata& AssetRegistry::Get(const AssetHandle handle) const
	{
		return m_LoadedAssets.at(handle);
	}

	size_t AssetRegistry::Count() const
	{
		return m_LoadedAssets.size();
	}

	bool AssetRegistry::Contains(AssetHandle handle) const
	{
		return m_LoadedAssets.contains(handle);
	}

	std::unordered_map<AssetHandle, AssetMetadata>::iterator AssetRegistry::Find(AssetHandle handle)
	{
		return m_LoadedAssets.find(handle);
	}

    const std::unordered_map<AssetHandle, AssetMetadata>::const_iterator AssetRegistry::Find(AssetHandle handle) const
    {
		return m_LoadedAssets.find(handle);
    }

	size_t AssetRegistry::Remove(AssetHandle handle)
	{
		VX_CORE_ASSERT(Contains(handle), "Registry doesn't contain asset handle!");
		return m_LoadedAssets.erase(handle);
	}

	void AssetRegistry::Clear()
	{
		m_LoadedAssets.clear();
	}

}
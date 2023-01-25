#include "vxpch.h"
#include "InstancePool.h"

namespace Vortex {

	void InstancePool::AddInstance(void* instance)
	{
		std::lock_guard<std::mutex> lock(m_InstancePoolMutex);
		VX_CORE_ASSERT(instance, "Invalid Instance!");
		m_Pool.insert(instance);
	}

	size_t InstancePool::RemoveInstance(void* instance)
	{
		std::lock_guard<std::mutex> lock(m_InstancePoolMutex);
		VX_CORE_ASSERT(Contains(instance), "Invalid Instance!");
		return m_Pool.erase(instance);
	}

	bool InstancePool::Contains(void* instance)
	{
		return m_Pool.contains(instance);
	}

}

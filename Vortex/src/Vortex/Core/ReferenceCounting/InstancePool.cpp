#include "vxpch.h"
#include "InstancePool.h"

namespace Vortex {

	void InstancePool::AddInstance(void* instance)
	{
		std::lock_guard<std::mutex> lock(m_InstancePoolMutex);
		VX_CORE_ASSERT(instance, "Invalid Instance!");
		m_Pool.insert(instance);
	}

	void InstancePool::RemoveInstance(void* instance)
	{
		VX_CORE_ASSERT(instance, "Invalid Instance!");

		if (!Contains(instance))
			return;

		std::lock_guard<std::mutex> lock(m_InstancePoolMutex);
		m_Pool.erase(instance);
	}

	bool InstancePool::Contains(void* instance)
	{
		VX_CORE_ASSERT(instance, "Invalid Instance!");
		return m_Pool.contains(instance);
	}

}

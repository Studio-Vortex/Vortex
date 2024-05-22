#pragma once

#include <unordered_set>
#include <mutex>

namespace Vortex {

	class InstancePool
	{
	public:
		InstancePool() = default;
		~InstancePool() = default;

		void AddInstance(void* instance);
		void RemoveInstance(void* instance);
		bool Contains(void* instance);

	private:
		std::unordered_set<void*> m_Pool;
		std::mutex m_InstancePoolMutex;
	};

}

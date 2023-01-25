#pragma once

#include "Vortex/Core/Base.h"

#include <atomic>

namespace Vortex {

	class RefCounted
	{
	public:
		VX_FORCE_INLINE void IncRefCount() const
		{
			++m_RefCount;
		}

		VX_FORCE_INLINE void DecRefCount() const
		{
			--m_RefCount;
		}

		VX_FORCE_INLINE uint32_t GetRefCount() const
		{
			return m_RefCount.load();
		}

	private:
		mutable std::atomic<uint32_t> m_RefCount = 0;
	};

}

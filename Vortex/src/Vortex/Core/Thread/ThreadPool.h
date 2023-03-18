#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/Thread/Thread.h"

#include <bitset>

namespace Vortex {

	constexpr static uint32_t MAX_THREADS = 50;

	class VORTEX_API ThreadPool
	{
	public:
		static void Init();
		static void Shutdown();

		static Thread* GetNextThread();
		static void SubmitThreadFree(Thread* thread);

	private:
		inline static Thread** s_ThreadPool = nullptr;
		inline static std::bitset<MAX_THREADS> s_FreeThreads;
	};

}

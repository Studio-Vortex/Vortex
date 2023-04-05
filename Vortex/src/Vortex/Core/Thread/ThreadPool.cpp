#include "vxpch.h"
#include "ThreadPool.h"

namespace Vortex {

	void ThreadPool::Init()
	{
		Thread* threadPool = new Thread[MAX_THREADS];
		s_ThreadPool = &threadPool;
		s_FreeThreads.reset();
	}

	void ThreadPool::Shutdown()
	{
		for (uint32_t i = 0; i < MAX_THREADS; i++)
		{
			if (!s_FreeThreads[i])
				SubmitThreadFree(s_ThreadPool[i]);
		}

		Thread* threadPool = *s_ThreadPool;
		delete[] threadPool;
		s_FreeThreads.reset();
	}

	Thread* ThreadPool::GetNextThread()
	{
		size_t threadIndex = 0;
		Thread* result = nullptr;

		for (uint32_t i = 0; i < MAX_THREADS; i++)
		{
			if (s_FreeThreads[i])
			{
				threadIndex = i;
			}
		}

		result = s_ThreadPool[threadIndex];

		s_FreeThreads.set(threadIndex, true);

		return result;
	}

	void ThreadPool::SubmitThreadFree(Thread* thread)
	{
		for (uint32_t i = 0; i < MAX_THREADS; i++)
		{
			if (thread != s_ThreadPool[i])
				continue;
			
			s_FreeThreads[i].flip();

			break;
		}
	}

}

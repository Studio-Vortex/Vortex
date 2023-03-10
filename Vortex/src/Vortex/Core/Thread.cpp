#include "vxpch.h"
#include "Thread.h"

namespace Vortex {

	Thread::Thread(const ThreadFn& func)
		: m_Thread(func)
	{
		
	}

	Thread::~Thread()
	{

	}

	void Thread::Join()
	{
		m_Thread.join();
	}

}

#include "vxpch.h"
#include "Thread.h"

namespace Vortex {

	Thread::Thread(const LaunchFn& fn)
		: m_Thread(fn)
	{
	}

	Thread::~Thread()
	{
		
	}

	bool Thread::Joinable() const
	{
		return m_Thread.joinable();
	}

	void Thread::Join()
	{
		m_Thread.join();
	}

}

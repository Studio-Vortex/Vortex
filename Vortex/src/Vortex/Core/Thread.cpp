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

    void Thread::Detach()
    {
		m_Thread.detach();
    }

	Thread::ID Thread::GetID() const
	{
		return m_Thread.get_id();
	}

	SharedReference<Thread> Thread::Create(const LaunchFn& fn)
	{
		return SharedReference<Thread>::Create(fn);
	}

}

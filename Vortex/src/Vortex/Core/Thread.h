#pragma once

#include "Vortex/Core/Base.h"

#include <functional>
#include <thread>

namespace Vortex {

	class VORTEX_API Thread
	{
	public:
		using ThreadFn = std::function<void()>;

	public:
		Thread() = default;
		Thread(const ThreadFn& func);
		~Thread();

		void Join();

	private:
		std::thread m_Thread;
	};

}

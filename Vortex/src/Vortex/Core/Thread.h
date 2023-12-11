#pragma once

#include "Vortex/Core/Base.h"

#include <functional>
#include <thread>

namespace Vortex {

	class VORTEX_API Thread
	{
	public:
		using Fn = std::function<void()>;
	public:
		Thread() = default;
		Thread(const Fn& fn);
		~Thread();

		bool Joinable() const;
		void Join();

	private:
		std::thread m_Thread;
	};

}

#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/ReferenceCounting/SharedRef.h"
#include "Vortex/ReferenceCounting/RefCounted.h"

#include <functional>
#include <thread>

namespace Vortex {

	class VORTEX_API Thread : public RefCounted
	{
	public:
		using VORTEX_API LaunchFn = std::function<void()>;
		using VORTEX_API ID = std::thread::id;

	public:
		Thread() = default;
		Thread(const LaunchFn& fn);
		~Thread();

		bool Joinable() const;
		void Join();

		void Detach();

		ID GetID() const;

		static SharedReference<Thread> Create(const LaunchFn& fn);

	private:
		std::thread m_Thread;
	};

}

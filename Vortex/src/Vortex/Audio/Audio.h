#pragma once

#include "Vortex/Core/Base.h"

namespace Wave {

	class Context;

}

namespace Vortex {

	class VORTEX_API Audio
	{
	public:
		static void Init();
		static void Shutdown();

		static SharedRef<Wave::Context> GetContext();
	};

}

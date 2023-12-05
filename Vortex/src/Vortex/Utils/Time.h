#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/TimeStep.h"

namespace Vortex {

	class VORTEX_API Time
	{
	public:
		static float GetTime();
		static TimeStep GetDeltaTime();

		// Internal use only

		static void SetDeltaTime(TimeStep delta);
	};

}

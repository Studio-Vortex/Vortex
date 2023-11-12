#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	struct VORTEX_API Version
	{
		uint32_t Major;
		uint32_t Minor;
		uint32_t Patch;

		Version(uint32_t major = 0, uint32_t minor = 0, uint32_t patch = 0)
			: Major(major), Minor(minor), Patch(patch) { }
	};

}

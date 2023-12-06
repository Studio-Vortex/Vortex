#pragma once

#include "Vortex/Core/Base.h"

#include <cstdint>

namespace Vortex {

	struct VORTEX_API MonoAssemblyTypeInfo
	{
		const char* Namespace = "";
		const char* Name = "";
		uint32_t FieldCount = 0;
	};

}

#pragma once

#include <cstdint>

namespace Vortex {

	struct MonoAssemblyTypeInfo
	{
		const char* Namespace = "";
		const char* Name = "";
		uint32_t FieldCount = 0;
	};

}

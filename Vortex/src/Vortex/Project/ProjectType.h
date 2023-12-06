#pragma once

#include "Vortex/Core/Base.h"

#include <cstdint>

namespace Vortex {

	enum class VORTEX_API ProjectType : int32_t
	{
		e2D, e3D
	};

	namespace Utils {

		ProjectType ProjectTypeFromString(const char* type);
		const char* ProjectTypeToString(ProjectType type);

	}

}

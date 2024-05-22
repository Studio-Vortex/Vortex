#pragma once

#include "Vortex/Core/Base.h"

#include <cstdint>
#include <string>

namespace Vortex {

	enum class VORTEX_API SystemType : uint16_t
	{
		None = 0,
		UI,
	};

	namespace Utils {

		SystemType SystemTypeFromString(const std::string& type);
		std::string StringFromSystemType(SystemType type);

	}

}

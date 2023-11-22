#pragma once

#include <cstdint>

namespace Vortex {

	enum class AttenuationModel : uint8_t
	{
		None = 0, Inverse, Linear, Exponential,
	};

}

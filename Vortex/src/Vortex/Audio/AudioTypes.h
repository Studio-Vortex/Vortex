#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	enum class VORTEX_API AttenuationModel : uint8_t
	{
		None = 0, Inverse, Linear, Exponential,
	};

	enum class VORTEX_API PanMode : uint8_t
	{
		Balance = 0, Pan,
	};

	enum class VORTEX_API PositioningMode : uint8_t
	{
		Absolute = 0, Relative,
	};

}

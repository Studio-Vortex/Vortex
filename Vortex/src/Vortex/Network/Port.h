#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	struct VORTEX_API Port
	{
		uint16_t Address;

		Port(uint16_t port = 27999)
			: Address(port) { }
	};

}

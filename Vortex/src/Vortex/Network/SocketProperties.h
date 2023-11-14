#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Network/NetworkTypes.h"

namespace Vortex {

	struct VORTEX_API SocketProperties
	{
		AddressFamily AF;
		ConnectionType Type;
		NetworkProtocol Protocol;
	};

}
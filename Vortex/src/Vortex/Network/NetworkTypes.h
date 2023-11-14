#pragma once

#include "Vortex/Core/Base.h"

namespace Vortex {

	enum class VORTEX_API SocketOption
	{
		None = 0,
	};

	enum class VORTEX_API AddressFamily
	{
		None = 0, InternetIPv4, InternetIPv6,
	};

	enum class VORTEX_API ConnectionType
	{
		None = 0, Tcp, Udp,
	};

	enum class VORTEX_API NetworkProtocol
	{
		None = 0, Tcp, Udp,
	};

}

#pragma once

#include "Vortex/Network/NetworkTypes.h"

#include <string>

namespace Vortex {

	namespace Utils {

		int AddressFamilyToWinSockAF(AddressFamily family);
		int ConnectionTypeToWinSockType(ConnectionType type);
		int NetworkProtocolToWinSockProtocol(NetworkProtocol protocol);
		std::string SocketOptionToWinSockOption(SocketOption opt);

	}

}

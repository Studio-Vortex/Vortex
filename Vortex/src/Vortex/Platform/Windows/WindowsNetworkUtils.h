#pragma once

#include "Vortex/Network/NetworkTypes.h"

#include <string>

namespace Vortex::Utils {

	int NetworkChannelToWinSockChannel(NetworkChannel channel);
	int AddressFamilyToWinSockAF(AddressFamily family);
	int ConnectionTypeToWinSockType(ConnectionType type);
	int NetworkProtocolToWinSockProtocol(NetworkProtocol protocol);
	std::string SocketOptionToWinSockOption(SocketOption opt);

}

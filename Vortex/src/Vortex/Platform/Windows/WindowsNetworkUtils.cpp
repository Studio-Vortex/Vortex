#include "vxpch.h"
#include "WindowsNetworkUtils.h"

#include <WinSock2.h>

namespace Vortex::Utils {

	int AddressFamilyToWinSockAF(AddressFamily family)
	{
		switch (family)
		{
			case AddressFamily::None:     return 0;
			case AddressFamily::Internet: return AF_INET;
		}

		VX_CORE_ASSERT(false, "Unknown address family");
		return 0;
	}

	int ConnectionTypeToWinSockType(ConnectionType type)
	{
		switch (type)
		{
			case ConnectionType::None: return 0;
			case ConnectionType::Tcp:  return SOCK_STREAM;
			case ConnectionType::Udp:  return SOCK_RAW;
		}

		VX_CORE_ASSERT(false, "Unknown connection type");
		return 0;
	}

	int NetworkProtocolToWinSockProtocol(NetworkProtocol protocol)
	{
		switch (protocol)
		{
			case NetworkProtocol::None: return 0;
			case NetworkProtocol::Tcp:  return IPPROTO_TCP;
			case NetworkProtocol::Udp:  return IPPROTO_UDP;
		}

		VX_CORE_ASSERT(false, "Unknown network protocol");
		return 0;
	}

    std::string SocketOptionToWinSockOption(SocketOption opt)
    {
		switch (opt)
		{
		default:
			break;
		}

		VX_CORE_ASSERT(false, "Unknown socket option");
		return "None";
    }

}

#include "vxpch.h"
#include "Vortex/Network/Socket.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Platform/Windows/WindowsNetworkUtils.h"
#include "Vortex/Network/NetworkAssert.h"

#include <WinSock2.h>
#include <Windows.h>

#include <unordered_map>

namespace Vortex {

	struct SocketInternalData
	{
		std::unordered_map<UUID, SOCKET> ActiveSockets;
	};

	static SocketInternalData s_Data;

	namespace Utils {

		SOCKET GetSocket(UUID socketID)
		{
			VX_CORE_ASSERT(s_Data.ActiveSockets.contains(socketID), "Invalid socket ID");
			SOCKET sock = s_Data.ActiveSockets[socketID];
			VX_CORE_ASSERT(sock != INVALID_SOCKET, "Invalid socket");
			return sock;
		}

	}

	bool Socket::Init(AddressFamily family, ConnectionType type, NetworkProtocol protocol)
	{
		m_AF = family;
		m_Type = type;
		m_Protocol = protocol;

		SOCKET sock = socket(
			Utils::AddressFamilyToWinSockAF(m_AF),
			Utils::ConnectionTypeToWinSockType(m_Type),
			Utils::NetworkProtocolToWinSockProtocol(m_Protocol)
		);

		if (sock == INVALID_SOCKET)
		{
			return false;
		}

		m_SocketID = (UUID)sock;
		VX_CORE_ASSERT(!s_Data.ActiveSockets.contains(m_SocketID), "Socket with ID already exists");
		s_Data.ActiveSockets[m_SocketID] = sock;

		return true;
	}

	void Socket::Shutdown()
	{
		SOCKET sock = Utils::GetSocket(m_SocketID);

		VX_CHECK_NETWORK_RESULT(
			closesocket(sock),
			"Failed to close socket ID: '{}'",
			(uint32_t)m_SocketID
		);
	}

	bool Socket::IsValid() const
	{
		return s_Data.ActiveSockets.contains(m_SocketID)
			&& Utils::GetSocket(m_SocketID) != INVALID_SOCKET;
	}

	std::string Socket::GetOption(SocketOption opt) const
	{
		return "";
	}

	void Socket::SetOption(SocketOption opt, const std::string& value)
	{
		
	}

}

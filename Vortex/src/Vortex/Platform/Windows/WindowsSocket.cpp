#include "vxpch.h"
#include "Vortex/Platform/Windows/WindowsSocket.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Platform/Windows/WindowsNetworkUtils.h"
#include "Vortex/Network/NetworkAssert.h"

#include <WinSock2.h>
#include <Windows.h>

#include <unordered_map>

namespace Vortex {

	using Ws2Socket = SOCKET;

	struct SocketInternalData
	{
		std::unordered_map<UUID, Ws2Socket> ActiveSockets;
	};

	static SocketInternalData s_Data;

	namespace Utils {

		Ws2Socket GetSocket(UUID socketID)
		{
			VX_CORE_ASSERT(s_Data.ActiveSockets.contains(socketID), "Invalid socket ID");
			Ws2Socket sock = s_Data.ActiveSockets[socketID];
			VX_CORE_ASSERT(sock != INVALID_SOCKET, "Invalid socket");
			return sock;
		}

	}

	WindowsSocket::WindowsSocket(const SocketProperties& socketProps)
		: m_Properties(socketProps), m_IpAddr(IpAddress::GetLoopbackAddr()), m_Port(27999)
	{
		Ws2Socket sock = socket(
			Utils::AddressFamilyToWinSockAF(m_Properties.AF),
			Utils::ConnectionTypeToWinSockType(m_Properties.Type),
			Utils::NetworkProtocolToWinSockProtocol(m_Properties.Protocol)
		);

		if (sock == INVALID_SOCKET)
		{
			VX_CORE_ASSERT(false, "Failed to create socket");
			return;
		}

		m_SocketID = (UUID)sock;
		VX_CORE_ASSERT(!s_Data.ActiveSockets.contains(m_SocketID), "Socket with ID already exists");
		s_Data.ActiveSockets[m_SocketID] = sock;

#ifdef VX_DEBUG
		VX_CONSOLE_LOG_INFO("Socket opened ID: '{}'", m_SocketID);
#endif // VX_DEBUG
	}

	WindowsSocket::~WindowsSocket() { }

	void WindowsSocket::Shutdown()
	{
		Ws2Socket sock = Utils::GetSocket(m_SocketID);

		VX_CHECK_NETWORK_RESULT(
			closesocket(sock),
			"Failed to close socket ID: '{}'",
			m_SocketID
		);

#ifdef VX_DEBUG
		VX_CONSOLE_LOG_INFO("Socket closed ID: '{}'", m_SocketID);
#endif // VX_DEBUG
	}

	void WindowsSocket::Bind(Port port, IpAddress ipAddr)
	{
		Ws2Socket sock = Utils::GetSocket(m_SocketID);

		SOCKADDR_IN serverInfo;
		ZeroMemory(&serverInfo, sizeof(SOCKADDR_IN));

		serverInfo.sin_family = Utils::AddressFamilyToWinSockAF(m_Properties.AF);
		serverInfo.sin_port = htons(port.Address);
		std::string addrStr = ipAddr.ToString();
		memcpy(&serverInfo.sin_addr.S_un.S_addr, addrStr.c_str(), sizeof(serverInfo.sin_addr.S_un.S_addr));

		VX_CHECK_NETWORK_RESULT(
			bind(sock, (const sockaddr*)&serverInfo, sizeof(serverInfo)),
			"Failed to bind socket ID: '{}'",
			m_SocketID
		);

#ifdef VX_DEBUG
		VX_CONSOLE_LOG_INFO("Socket ID: '{}' binded to port: '{}' at ip: '{}'", m_SocketID, port.Address, addrStr);
#endif // VX_DEBUG
	}

	void WindowsSocket::Listen()
	{
	}

	void WindowsSocket::Accept()
	{
	}

	void WindowsSocket::Connect()
	{
	}

	void WindowsSocket::Receive()
	{
	}

	void WindowsSocket::Send()
	{
	}

	bool WindowsSocket::IsValid() const
	{
		return s_Data.ActiveSockets.contains(m_SocketID)
			&& Utils::GetSocket(m_SocketID) != INVALID_SOCKET;
	}

	std::string WindowsSocket::GetOption(SocketOption opt) const
	{
		return "";
	}

	void WindowsSocket::SetOption(SocketOption opt, const std::string& value)
	{
		
	}

}

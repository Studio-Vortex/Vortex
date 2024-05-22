#include "vxpch.h"
#include "Vortex/Platform/Windows/WindowsSocket.h"

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/String.h"

#include "Vortex/Platform/Windows/WindowsNetworkUtils.h"
#include "Vortex/Network/NetworkAssert.h"

#include <WinSock2.h>
#include <Windows.h>

#include <unordered_map>

namespace Vortex {

#define DEBUG_CONFIG VX_RELEASE

	using Ws2Socket = SOCKET;

	struct SocketInternalData
	{
		std::unordered_map<UUID, Ws2Socket> ActiveSockets;
	};

	static SocketInternalData s_Data;

	namespace Utils {

		bool SocketExists(UUID socketID)
		{
			return s_Data.ActiveSockets.contains(socketID);
		}

		void AddSocket(UUID socketID, Ws2Socket sock)
		{
			VX_CORE_ASSERT(!SocketExists(socketID), "Socket with ID already exists");
			s_Data.ActiveSockets[socketID] = sock;
		}

		void RemoveSocket(UUID socketID)
		{
			VX_CORE_ASSERT(SocketExists(socketID), "Invalid socket ID");
			auto it = s_Data.ActiveSockets.find(socketID);
			s_Data.ActiveSockets.erase(it);
		}

		Ws2Socket GetSocket(UUID socketID)
		{
			VX_CORE_ASSERT(SocketExists(socketID), "Invalid socket ID");
			Ws2Socket sock = s_Data.ActiveSockets[socketID];
			VX_CORE_ASSERT(sock != INVALID_SOCKET, "Invalid socket");
			return sock;
		}

	}

	WindowsSocket::WindowsSocket(const SocketProperties& socketProps, UUID socketID)
		: m_Properties(socketProps)
	{
		Ws2Socket sock = INVALID_SOCKET;

		if (socketID != 0)
		{
			m_SocketID = socketID;
			sock = (Ws2Socket)m_SocketID;

			if (!Utils::SocketExists(m_SocketID))
			{
				Utils::AddSocket(m_SocketID, sock);
			}
		}
		else
		{
			sock = socket(
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
			Utils::AddSocket(m_SocketID, sock);
		}

#ifdef DEBUG_CONFIG
		VX_CONSOLE_LOG_INFO("Socket({}) created", m_SocketID);
#endif // DEBUG_CONFIG
	}

	WindowsSocket::~WindowsSocket()
	{
		if (TestConnection())
		{
			Disconnect(NetworkChannel::Both);
		}
	}

	bool WindowsSocket::Disconnect(NetworkChannel channel)
	{
		if (!Utils::SocketExists(m_SocketID))
			return false;

		Ws2Socket sock = Utils::GetSocket(m_SocketID);

		VX_CHECK_NETWORK_RESULT(
			shutdown(sock, Utils::NetworkChannelToWinSockChannel(channel)),
			"Failed to disconnect socket({})", m_SocketID
		);

		VX_CHECK_NETWORK_RESULT(
			closesocket(sock),
			"Failed to close socket({})", m_SocketID
		);

#ifdef DEBUG_CONFIG
		VX_CONSOLE_LOG_INFO("Socket({}) closed", m_SocketID);
#endif // DEBUG_CONFIG

		Utils::RemoveSocket(m_SocketID);
	}

	bool WindowsSocket::Bind()
	{
		if (!Utils::SocketExists(m_SocketID))
			return false;

		Ws2Socket sock = Utils::GetSocket(m_SocketID);

		SOCKADDR_IN serverInfo;
		ZeroMemory(&serverInfo, sizeof(SOCKADDR_IN));

		serverInfo.sin_family = Utils::AddressFamilyToWinSockAF(m_Properties.AF);
		serverInfo.sin_port = htons(m_Properties.PortAddr.Address);
		std::string addrStr = m_Properties.IpAddr.ToString();
		serverInfo.sin_addr.S_un.S_addr = inet_addr(addrStr.c_str());

		VX_CHECK_NETWORK_RESULT(
			bind(sock, (const sockaddr*)&serverInfo, sizeof(serverInfo)),
			"Failed to bind socket({})", m_SocketID
		);

#ifdef DEBUG_CONFIG
		VX_CONSOLE_LOG_INFO("Socket({}) listening @ '{}:{}'", m_SocketID, addrStr, m_Properties.PortAddr.Address);
#endif // DEBUG_CONFIG

		return true;
	}

	bool WindowsSocket::Listen()
	{
		int32_t listenResult = listen(Utils::GetSocket(m_SocketID), SOMAXCONN);
		if (listenResult == SOCKET_ERROR)
		{
			return false;
		}

		return true;
	}

	SharedReference<Socket> WindowsSocket::Accept()
	{
		Ws2Socket clientSocket = accept(Utils::GetSocket(m_SocketID), NULL, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			return nullptr;
		}

		return WindowsSocket::Create(m_Properties, (UUID)clientSocket);
	}

	bool WindowsSocket::Connect()
	{
		return true;
	}

	bool WindowsSocket::Receive(Buffer& recvPacket)
	{
		return true;
	}

	bool WindowsSocket::Send(const Buffer& sendPacket)
	{
		return true;
	}

	bool WindowsSocket::TestConnection() const
	{
		return Utils::SocketExists(m_SocketID) && Utils::GetSocket(m_SocketID) != INVALID_SOCKET;
	}

	std::string WindowsSocket::GetOption(SocketOption opt) const
	{
		return "";
	}

	void WindowsSocket::SetOption(SocketOption opt, const std::string& value)
	{
		
	}

}

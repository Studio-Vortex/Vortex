#include "vxpch.h"
#include "Vortex/Platform/Windows/WindowsServer.h"

#include "Vortex/Network/Socket.h"

namespace Vortex {

#define DEBUG_CONFIG VX_RELEASE

	WindowsServer::WindowsServer(const ServerProperties& serverProps)
		: m_Properties(serverProps)
	{
		m_Socket = Socket::Create(m_Properties.SocketOptions);

#ifdef DEBUG_CONFIG
		std::string addrStr = m_Properties.IpAddr.ToString();
		uint16_t port = m_Properties.PortAddr.Address;
		VX_CONSOLE_LOG_INFO("Server started at '{}' on port '{}'", addrStr, port);
#endif // DEBUG_CONFIG
	}

	WindowsServer::~WindowsServer() { }

	void WindowsServer::Shutdown()
	{
		m_Socket->Disconnect(NetworkChannel::Both);

#ifdef DEBUG_CONFIG
		std::string addrStr = m_Properties.IpAddr.ToString();
		uint16_t port = m_Properties.PortAddr.Address;
		VX_CONSOLE_LOG_INFO("Server shutting down at '{}' on port '{}'", addrStr, port);
#endif // DEBUG_CONFIG
	}

	void WindowsServer::Bind()
	{
		m_Socket->Bind(m_Properties.PortAddr, m_Properties.IpAddr);
	}

	void WindowsServer::Listen()
	{
		m_Socket->Listen();
	}

	void WindowsServer::Accept()
	{
		m_Socket->Accept();
	}

	void WindowsServer::Connect()
	{
		m_Socket->Connect();
	}

	void WindowsServer::Receive()
	{
		m_Socket->Receive();
	}

	void WindowsServer::Send()
	{
		m_Socket->Send();
	}

}

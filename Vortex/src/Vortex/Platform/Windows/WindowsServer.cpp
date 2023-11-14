#include "vxpch.h"
#include "Vortex/Platform/Windows/WindowsServer.h"

#include "Vortex/Network/Socket.h"

namespace Vortex {

	WindowsServer::WindowsServer(const ServerProperties& serverProps)
		: m_Properties(serverProps)
	{
		m_Socket = Socket::Create(serverProps.SocketOptions);
	}

	WindowsServer::~WindowsServer()
	{
		m_Socket->Shutdown();
	}

	void WindowsServer::Bind()
	{
		m_Socket->Bind();
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

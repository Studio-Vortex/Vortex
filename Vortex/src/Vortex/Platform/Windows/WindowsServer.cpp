#include "vxpch.h"
#include "Vortex/Platform/Windows/WindowsServer.h"

#include "Vortex/Network/Socket.h"

namespace Vortex {

#define DEBUG_CONFIG VX_RELEASE

	WindowsServer::WindowsServer(const ServerProperties& serverProps)
		: m_Properties(serverProps)
	{
	}

	WindowsServer::~WindowsServer()
	{
		if (m_IsRunning)
		{
			Shutdown();
		}
	}

	bool WindowsServer::Launch()
	{
		m_ServerSocket = Socket::Create(m_Properties.SocketOptions);
		m_IsRunning = true;

		m_ServerThread = Thread::Create(VX_BIND_CALLBACK(Run));
		m_ServerThread->Detach();
		return true;
	}

	void WindowsServer::Stop()
	{
		m_IsRunning = false;
	}

	bool WindowsServer::Shutdown()
	{
		bool result = m_ServerSocket->Disconnect(NetworkChannel::Both);

#ifdef DEBUG_CONFIG
		std::string addrStr = m_Properties.SocketOptions.IpAddr.ToString();
		VX_CONSOLE_LOG_INFO("Server shutting down @ '{}:{}'", addrStr, m_Properties.SocketOptions.PortAddr.Address);
#endif // DEBUG_CONFIG

		return result;
	}

	void WindowsServer::Run()
	{
		Listen();

#ifdef DEBUG_CONFIG
		std::string addrStr = m_Properties.SocketOptions.IpAddr.ToString();
		VX_CONSOLE_LOG_INFO("Server listening @ '{}:{}'", addrStr, m_Properties.SocketOptions.PortAddr.Address);
#endif // DEBUG_CONFIG

		while (m_IsRunning)
		{
			SharedReference<Socket> clientSocket = Accept();
			if (clientSocket == nullptr)
			{
				continue;
			}

			VX_CONSOLE_LOG_INFO("Client({}) connected", clientSocket->GetUUID());
			// TODO
		}
	}

	bool WindowsServer::Bind()
	{
		return m_ServerSocket->Bind();
	}

	bool WindowsServer::Listen()
	{
		return m_ServerSocket->Listen();
	}

	SharedReference<Socket> WindowsServer::Accept()
	{
		return m_ServerSocket->Accept();
	}

	bool WindowsServer::Connect()
	{
		return m_ServerSocket->Connect();
	}

	bool WindowsServer::Receive(Buffer& recvPacket)
	{
		return m_ServerSocket->Receive(recvPacket);
	}

	bool WindowsServer::Send(const Buffer& sendPacket)
	{
		return m_ServerSocket->Send(sendPacket);
	}

}

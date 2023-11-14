#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Network/Server.h"

namespace Vortex {

	class Socket;

	class WindowsServer : public Server
	{
	public:
		WindowsServer(const ServerProperties& serverProps);
		~WindowsServer() override;

		void Bind() override;
		void Listen() override;

		void Accept() override;
		void Connect() override;

		void Receive() override;
		void Send() override;

		const IpAddress& GetIpAddr() const override { return m_IpAddr; }
		Port GetPort() const override { return m_Port; }

	private:
		ServerProperties m_Properties;
		SharedReference<Socket> m_Socket = nullptr;
		IpAddress m_IpAddr;
		Port m_Port;
	};

}
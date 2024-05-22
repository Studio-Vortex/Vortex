#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Thread.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include "Vortex/Network/Server.h"

#include <unordered_map>

namespace Vortex {

	class Socket;

	class WindowsServer : public Server
	{
	public:
		WindowsServer(const ServerProperties& serverProps);
		~WindowsServer() override;

		bool Launch() override;
		void Stop() override;

		bool IsRunning() const override { return m_IsRunning; }

		const ServerProperties& GetProperties() const override { return m_Properties; }

	private:
		bool Shutdown() override;

		void Run();

		bool Bind() override;
		bool Listen() override;

		SharedReference<Socket> Accept() override;
		bool Connect() override;

		bool Receive(Buffer& recvPacket) override;
		bool Send(const Buffer& sendPacket) override;

	private:
		ServerProperties m_Properties;
		SharedReference<Socket> m_ServerSocket = nullptr;

		SharedReference<Thread> m_ServerThread;
		std::unordered_map<UUID, SharedReference<Socket>> m_ConnectedClients;

		bool m_IsRunning = false;
	};

}
#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Network/Socket.h"

namespace Vortex {

	class WindowsSocket : public Socket
	{
	public:
		WindowsSocket(const SocketProperties& socketProps, UUID socketID = 0);
		~WindowsSocket() override;

		bool Disconnect(NetworkChannel channel) override;

		bool Bind() override;
		bool Listen() override;

		SharedReference<Socket> Accept() override;
		bool Connect() override;

		bool Receive(Buffer& recvPacket) override;
		bool Send(const Buffer& sendPacket) override;

		UUID GetUUID() const override { return m_SocketID; }
		const SocketProperties& GetProperties() const override { return m_Properties; }

		bool TestConnection() const override;

		std::string GetOption(SocketOption opt) const override;
		void SetOption(SocketOption opt, const std::string& value) override;

	private:
		SocketProperties m_Properties;
		UUID m_SocketID;
	};

}

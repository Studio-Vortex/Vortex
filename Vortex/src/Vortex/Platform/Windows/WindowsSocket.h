#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Network/Socket.h"

namespace Vortex {

	class WindowsSocket : public Socket
	{
	public:
		WindowsSocket(const SocketProperties& socketProps);
		~WindowsSocket() override;

		void Shutdown() override;

		void Bind(Port port, IpAddress ipAddr) override;
		void Listen() override;

		void Accept() override;
		void Connect() override;

		void Receive() override;
		void Send() override;

		UUID GetUUID() const override { return m_SocketID; }
		AddressFamily GetAddressFamily() const override { return m_Properties.AF; }
		ConnectionType GetConnectionType() const override { return m_Properties.Type; }
		NetworkProtocol GetNetworkProtocol() const override { return m_Properties.Protocol; }

		const IpAddress& GetIpAddr() const override { return m_IpAddr; }
		Port GetPort() const override { return m_Port; }

		bool IsValid() const override;

		std::string GetOption(SocketOption opt) const override;
		void SetOption(SocketOption opt, const std::string& value) override;

	private:
		SocketProperties m_Properties;
		UUID m_SocketID;

		IpAddress m_IpAddr;
		Port m_Port;
	};

}

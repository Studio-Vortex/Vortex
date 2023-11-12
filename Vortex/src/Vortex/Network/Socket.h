#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Network/NetworkTypes.h"

#include <string>

namespace Vortex {

	class VORTEX_API Socket
	{
	public:
		bool Init(AddressFamily family, ConnectionType type, NetworkProtocol protocol);
		void Shutdown();

		UUID GetUUID() const { return m_SocketID; }
		AddressFamily GetAddressFamily() const { return m_AF; }
		ConnectionType GetConnectionType() const { return m_Type; }
		NetworkProtocol GetNetworkProtocol() const { return m_Protocol; }

		bool IsValid() const;

		std::string GetOption(SocketOption opt) const;
		void SetOption(SocketOption opt, const std::string& value);

	private:
		UUID m_SocketID;
		AddressFamily m_AF;
		ConnectionType m_Type;
		NetworkProtocol m_Protocol;
	};

}

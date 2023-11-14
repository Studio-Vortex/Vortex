#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Core/ReferenceCounting/SharedRef.h"
#include "Vortex/Core/ReferenceCounting/RefCounted.h"

#include "Vortex/Network/Port.h"
#include "Vortex/Network/IpAddress.h"
#include "Vortex/Network/NetworkTypes.h"
#include "Vortex/Network/SocketProperties.h"

#include <string>

namespace Vortex {

	class VORTEX_API Socket : public RefCounted
	{
	public:
		virtual ~Socket() = default;

		virtual void Disconnect(NetworkChannel channel) = 0;

		virtual void Bind(Port port, IpAddress ipAddr) = 0;
		virtual void Listen() = 0;

		virtual void Accept() = 0;
		virtual void Connect() = 0;

		virtual void Receive() = 0;
		virtual void Send() = 0;

		virtual UUID GetUUID() const = 0;
		virtual AddressFamily GetAddressFamily() const = 0;
		virtual ConnectionType GetConnectionType() const = 0;
		virtual NetworkProtocol GetNetworkProtocol() const = 0;

		virtual const IpAddress& GetIpAddr() const = 0;
		virtual Port GetPort() const = 0;

		virtual bool IsValid() const = 0;

		virtual std::string GetOption(SocketOption opt) const = 0;
		virtual void SetOption(SocketOption opt, const std::string& value) = 0;

		static SharedReference<Socket> Create(const SocketProperties& socketProps);
	};

}

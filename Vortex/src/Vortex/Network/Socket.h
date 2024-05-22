#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/Buffer.h"

#include "Vortex/ReferenceCounting/SharedRef.h"
#include "Vortex/ReferenceCounting/RefCounted.h"

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

		virtual bool Disconnect(NetworkChannel channel) = 0;

		virtual bool Bind() = 0;
		virtual bool Listen() = 0;

		virtual SharedReference<Socket> Accept() = 0;
		virtual bool Connect() = 0;

		virtual bool Receive(Buffer& recvPacket) = 0;
		virtual bool Send(const Buffer& sendPacket) = 0;

		virtual UUID GetUUID() const = 0;
		virtual const SocketProperties& GetProperties() const = 0;

		virtual bool TestConnection() const = 0;

		virtual std::string GetOption(SocketOption opt) const = 0;
		virtual void SetOption(SocketOption opt, const std::string& value) = 0;

		static SharedReference<Socket> Create(const SocketProperties& socketProps, UUID socketID = 0);
	};

}

#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/Buffer.h"

#include "Vortex/Network/Port.h"
#include "Vortex/Network/IpAddress.h"
#include "Vortex/Network/NetworkTypes.h"
#include "Vortex/Network/Socket.h"
#include "Vortex/Network/SocketProperties.h"

#include "Vortex/ReferenceCounting/SharedRef.h"
#include "Vortex/ReferenceCounting/RefCounted.h"

namespace Vortex {

	struct VORTEX_API ServerProperties
	{
		SocketProperties SocketOptions;
	};

	class VORTEX_API Server : public RefCounted
	{
	public:
		virtual ~Server() = default;

		virtual bool Launch() = 0;
		virtual void Stop() = 0;

		virtual bool IsRunning() const = 0;

		virtual const ServerProperties& GetProperties() const = 0;

		static SharedReference<Server> Create(const ServerProperties& serverProps);

	private:
		virtual bool Shutdown() = 0;

		virtual bool Bind() = 0;
		virtual bool Listen() = 0;

		virtual SharedReference<Socket> Accept() = 0;
		virtual bool Connect() = 0;

		virtual bool Receive(Buffer& recvPacket) = 0;
		virtual bool Send(const Buffer& sendPacket) = 0;
	};

}

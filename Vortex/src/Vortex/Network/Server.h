#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Network/Port.h"
#include "Vortex/Network/IpAddress.h"
#include "Vortex/Network/NetworkTypes.h"
#include "Vortex/Network/SocketProperties.h"

#include "Vortex/ReferenceCounting/SharedRef.h"
#include "Vortex/ReferenceCounting/RefCounted.h"

namespace Vortex {

	struct VORTEX_API ServerProperties
	{
		SocketProperties SocketOptions;
		IpAddress IpAddr;
		Port PortAddr;
	};

	class VORTEX_API Server : public RefCounted
	{
	public:
		virtual ~Server() = default;

		virtual void Shutdown() = 0;

		virtual void Bind() = 0;
		virtual void Listen() = 0;

		virtual void Accept() = 0;
		virtual void Connect() = 0;

		virtual void Receive() = 0;
		virtual void Send() = 0;

		virtual const IpAddress& GetIpAddr() const = 0;
		virtual Port GetPort() const = 0;

		static SharedReference<Server> Create(const ServerProperties& serverProps);
	};

}

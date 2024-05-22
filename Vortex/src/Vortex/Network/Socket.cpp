#include "vxpch.h"
#include "Socket.h"

#ifdef VX_PLATFORM_WINDOWS
	#include "Vortex/Platform/Windows/WindowsSocket.h"
#endif // VX_PLATFORM_WINDOWS

namespace Vortex {

	SharedReference<Socket> Socket::Create(const SocketProperties& socketProps, UUID socketID)
	{
#ifdef VX_PLATFORM_WINDOWS
		return SharedReference<WindowsSocket>::Create(socketProps, socketID);
#endif // VX_PLATFORM_WINDOWS
	}

}

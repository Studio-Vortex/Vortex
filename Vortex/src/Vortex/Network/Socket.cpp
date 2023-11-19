#include "vxpch.h"
#include "Socket.h"

#ifdef VX_PLATFORM_WINDOWS
	#include "Vortex/Platform/Windows/WindowsSocket.h"
#endif // VX_PLATFORM_WINDOWS

namespace Vortex {

	SharedReference<Socket> Vortex::Socket::Create(const SocketProperties& socketProps)
	{
#ifdef VX_PLATFORM_WINDOWS
		return SharedReference<WindowsSocket>::Create(socketProps);
#endif // VX_PLATFORM_WINDOWS
	}

}

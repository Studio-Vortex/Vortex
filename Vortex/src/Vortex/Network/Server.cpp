#include "vxpch.h"
#include "Server.h"

#ifdef VX_PLATFORM_WINDOWS
	#include "Vortex/Platform/Windows/WindowsServer.h"
#endif // VX_PLATFORM_WINDOWS

namespace Vortex {

	SharedReference<Server> Vortex::Server::Create(const ServerProperties& serverProps)
	{
#ifdef VX_PLATFORM_WINDOWS
		return SharedReference<WindowsServer>::Create(serverProps);
#endif // VX_PLATFORM_WINDOWS
	}

}

#include "vxpch.h"
#include "Vortex/Networking/Networking.h"

#include <WinSock2.h>

#include "Vortex/Networking/NetworkAssert.h"

namespace Vortex {

#define WINSOCK_VERSION_MAJOR 2
#define WINSOCK_VERSION_MINOR 2

	void Networking::Init()
	{
		WSADATA wsa;
		ZeroMemory(&wsa, sizeof(WSADATA));
	}

	void Networking::Shutdown()
	{

	}

}

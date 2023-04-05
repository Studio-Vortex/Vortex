#include "vxpch.h"
#include "Vortex/Networking/Networking.h"

#include <WinSock2.h>
#include <ws2tcpip.h>

namespace Vortex {

	void Networking::Init()
	{
		WSADATA socketData;
		ZeroMemory(&socketData, sizeof(WSADATA));

		int result;

		// Init winsock
		result = WSAStartup(MAKEWORD(2, 2), &socketData);
		if (result)
		{

		}
	}

	void Networking::Shutdown()
	{

	}

}

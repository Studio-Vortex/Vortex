#include "sppch.h"
#include "WindowsNetworking.h"

namespace Sparky {

	WindowsSockets::WindowsSockets()
	{
		Init();
	}

	bool WindowsSockets::Init()
	{
		WSADATA socketData;
		ZeroMemory(&socketData, sizeof(WSADATA));

		int result;

		// Init winsock
		result = WSAStartup(MAKEWORD(2, 2), &socketData);
		if (result)
		{

		}

		return false;
	}

}

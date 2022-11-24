#pragma once

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

namespace Sparky {

	class WindowsSockets
	{
	public:
		WindowsSockets();

	private:
		bool Init();
	};

}

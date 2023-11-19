#include "vxpch.h"
#include "Vortex/Network/Networking.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

#include "Vortex/Network/NetworkAssert.h"

#include <WinSock2.h>
#include <Windows.h>

namespace Vortex {

#define WINSOCK_VERSION_MAJOR 2
#define WINSOCK_VERSION_MINOR 2

	struct NetworkingInternalData
	{
		WSADATA WinSockInfo;

		SubModule Module;
	};

	static NetworkingInternalData s_Data;

	void Networking::Init()
	{
		WORD dllVersion = MAKEWORD(WINSOCK_VERSION_MAJOR, WINSOCK_VERSION_MINOR);
		ZeroMemory(&s_Data.WinSockInfo, sizeof(WSADATA));

		VX_CHECK_NETWORK_RESULT(
			WSAStartup(dllVersion, &s_Data.WinSockInfo),
			"Failed to initialize WinSock"
		);

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Network";
		moduleProps.APIVersion = Version(1, 0, 0);
		moduleProps.RequiredModules = {};
		s_Data.Module.Init(moduleProps);

		Application::Get().AddModule(s_Data.Module);
	}

	void Networking::Shutdown()
	{
		VX_CHECK_NETWORK_RESULT(
			WSACleanup(),
			"Failed to shutdown WinSock"
		);

		Application::Get().RemoveModule(s_Data.Module);
		s_Data.Module.Shutdown();
	}

}

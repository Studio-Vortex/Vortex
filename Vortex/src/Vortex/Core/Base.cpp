#include "vxpch.h"
#include "Base.h"

#include "Vortex/Core/Log.h"

namespace Vortex {

	void InitalizeEngine()
	{
		// TODO: This should be reworked
		ShowWindow(GetConsoleWindow(), SW_HIDE);

		Log::Init();

		VX_CONSOLE_LOG_TRACE("Vortex Engine {}", VORTEX_BUILD_ID);
		VX_CONSOLE_LOG_TRACE("Initalizing...");
	}

	void ShutdownEngine()
	{
		VX_CONSOLE_LOG_TRACE("Shutting down...");
	}

}

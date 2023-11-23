#include "vxpch.h"
#include "Base.h"

#include "Vortex/Core/Log.h"
#include "Vortex/Core/Platform.h"

namespace Vortex {

	void InitalizeEngine()
	{
		Platform::HideConsoleWindow();

		Log::Init();

		VX_CONSOLE_LOG_TRACE("Vortex Engine {}", VORTEX_BUILD_ID);
		VX_CONSOLE_LOG_TRACE("Initalizing...");
	}

	void ShutdownEngine()
	{
		VX_CONSOLE_LOG_TRACE("Shutting down...");

		Log::Shutdown();
	}

}

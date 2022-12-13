#include "vxpch.h"
#include "Base.h"

#include "Vortex/Core/Log.h"

#define VORTEX_BUILD_ID "v0.1a"

namespace Vortex {

	void InitalizeEngine()
	{
		// TODO: This should be reworked
		ShowWindow(GetConsoleWindow(), SW_HIDE);

		Log::Init();

		VX_CORE_TRACE("Vortex Engine {}", VORTEX_BUILD_ID);
		VX_CORE_TRACE("Initalizing...");
	}

	void ShutdownEngine()
	{
		VX_CORE_TRACE("Shutting down...");
	}

}

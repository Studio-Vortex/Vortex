#include "sppch.h"
#include "Base.h"

#include "Sparky/Core/Log.h"

#define SPARKY_BUILD_ID "v0.1a"

namespace Sparky {

	void InitalizeEngine()
	{
		// TODO: This should be reworked
		ShowWindow(GetConsoleWindow(), SW_HIDE);

		Log::Init();

		SP_CORE_TRACE("Sparky Engine {}", SPARKY_BUILD_ID);
		SP_CORE_TRACE("Initalizing...");
	}

	void ShutdownEngine()
	{
		SP_CORE_TRACE("Shutting down...");
	}

}

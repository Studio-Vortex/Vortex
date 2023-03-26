#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	class VORTEX_API Platform
	{
	public:
		static void OpenURLInBrowser(const std::string& url);
		static std::string GetName();

		static void HideConsoleWindow();

		static void LaunchProcess(const char* binaryPath, const char* args);
	};

}

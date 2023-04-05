#include "vxpch.h"
#include "Vortex/Core/Platform.h"

#include <ShlObj.h>
#include <shellapi.h>

namespace Vortex {

	void Platform::OpenURLInBrowser(const std::string& url)
	{
		ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWDEFAULT);
	}

	std::string Platform::GetName()
	{
		return "Windows x64";
	}

    void Platform::HideConsoleWindow()
    {
		ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

	void Platform::LaunchProcess(const char* binaryPath, const char* args)
	{
		ShellExecuteA(NULL, "open", binaryPath, args, NULL, SWP_SHOWWINDOW);
	}

}

#include "vxpch.h"
#include "Vortex/Core/Platform.h"

#include <ShlObj.h>
#include <shellapi.h>

namespace Vortex {

	void Platform::OpenURLInBrowser(const std::string& url)
	{
		Platform::LaunchProcess(url.c_str(), "");
	}

	std::string Platform::GetName()
	{
		return "Windows x86_64";
	}

    void Platform::HideConsoleWindow()
    {
		ShowWindow(GetConsoleWindow(), SW_HIDE);
    }

	void Platform::LaunchProcess(const char* binaryPath, const char* args)
	{
		ShellExecuteA(nullptr, "open", binaryPath, args, nullptr, SWP_SHOWWINDOW);
	}

}

#include "sppch.h"
#include "Sparky/Utils/PlatformUtils.h"

#include "Sparky/Core/Application.h"

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <commdlg.h>
#include <shellapi.h>

namespace Sparky {

	std::string FileSystem::OpenFile(const char* fileFilter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = fileFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}
	
	std::string FileSystem::SaveFile(const char* fileFilter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetNativeWindowHandle());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = fileFilter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

	void FileSystem::OpenDirectory(const char* directoryName)
	{
		ShellExecuteA(NULL, "open", directoryName, NULL, NULL, SW_SHOWDEFAULT);
	}

	void FileSystem::LaunchApplication(const char* binaryPath, const char* args)
	{
		ShellExecuteA(NULL, "open", binaryPath, args, NULL, SWP_SHOWWINDOW);
	}

	void Random::Init()
	{
		s_RandomEngine.seed(std::random_device()());
	}

	float Random::Float()
	{
		return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
	}

	static float s_DeltaTime = 0.0f;

	float Time::GetTime()
	{
		return glfwGetTime();
	}

	float Time::GetDeltaTime()
	{
		return s_DeltaTime;
	}

	void Time::SetDeltaTime(float delta)
	{
		s_DeltaTime = delta;
	}

}

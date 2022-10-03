#pragma once

#include <string>

namespace Sparky {

	class FileSystem
	{
	public:
		static std::string OpenFile(const char* fileFilter);
		static std::string SaveFile(const char* fileFilter);
		static void OpenDirectory(const char* directoryName);
		static void LaunchApplication(const char* binaryPath, const char* args);
	};

	class Time
	{
	public:
		static float GetTime();
	};

}
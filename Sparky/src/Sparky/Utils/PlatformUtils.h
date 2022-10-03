#pragma once

#include <string>

namespace Sparky {

	class FileSystem
	{
	public:
		static std::string OpenFile(const char* fileFilter);
		static std::string SaveFile(const char* fileFilter);
		static void OpenDirectory(const char* directoryName);
	};

	class Time
	{
	public:
		static float GetTime();
	};

}
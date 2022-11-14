#pragma once

#include <string>
#include <random>

namespace Sparky {

	class FileSystem
	{
	public:
		static std::string OpenFile(const char* fileFilter);
		static std::string SaveFile(const char* fileFilter);
		static void OpenDirectory(const char* directoryName);
		static void LaunchApplication(const char* binaryPath, const char* args);
	};

	class Random
	{
	public:
		static void Init();
		static float Float();

		inline static std::mt19937 s_RandomEngine;
		inline static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};

	class Time
	{
	public:
		static float GetTime();
		static float GetDeltaTime();
		static void SetDeltaTime(float delta);
	};

}

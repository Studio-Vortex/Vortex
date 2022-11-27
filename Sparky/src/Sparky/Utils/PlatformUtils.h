#pragma once

#include "Sparky/Core/Buffer.h"

#include <string>
#include <random>

namespace Sparky {

	class FileSystem
	{
	public:
		static std::string OpenFileDialog(const char* fileFilter);
		static std::string SaveFileDialog(const char* fileFilter);
		static void OpenInFileExplorer(const char* directoryName);
		static Buffer ReadBinary(const std::filesystem::path& filepath);
		static std::string ReadText(const std::filesystem::path& filepath);
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

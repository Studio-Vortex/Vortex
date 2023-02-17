#pragma once

#include "Vortex/Core/Base.h"

#include <string>
#include <random>

namespace Vortex {

	class VORTEX_API FileDialogue
	{
	public:
		static std::string OpenFileDialog(const char* fileFilter);
		static std::string SaveFileDialog(const char* fileFilter);
		static std::string OpenFolderDialog(const char* initialFolder = "");
		static void OpenInFileExplorer(const char* directoryName);
	};

	class VORTEX_API Random
	{
	public:
		static void Init();
		static float Float();

		inline static std::mt19937 s_RandomEngine;
		inline static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};

	class VORTEX_API Time
	{
	public:
		static float GetTime();
		static float GetDeltaTime();
		static void SetDeltaTime(float delta);
	};

}

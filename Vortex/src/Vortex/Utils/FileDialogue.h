#pragma once

#include "Vortex/Core/Base.h"

#include <string>

namespace Vortex {

	class VORTEX_API FileDialogue
	{
	public:
		static std::string OpenFileDialog(const char* fileFilter);
		static std::string SaveFileDialog(const char* fileFilter);
		static std::string OpenFolderDialog(const char* initialFolder = "");
		static void OpenInFileExplorer(const char* directoryName);
	};

}
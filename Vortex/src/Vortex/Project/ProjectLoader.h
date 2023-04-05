#pragma once

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	class ProjectLoader
	{
	public:
		static bool LoadEditorProject(const std::filesystem::path& filepath);
		static bool SaveActiveEditorProject();

		static bool LoadRuntimeProject();

		static void CloseActiveProject();
	};

}

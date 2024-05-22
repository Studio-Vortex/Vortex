#pragma once

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	class ProjectLoader
	{
	public:
		static bool LoadEditorProject(const Fs::Path& filepath);
		static bool SaveActiveEditorProject();

		static bool LoadRuntimeProject();
	};

}

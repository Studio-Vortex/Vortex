#include "vxpch.h"
#include "ProjectLoader.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	bool ProjectLoader::LoadEditorProject(const std::filesystem::path& filepath)
	{
		if (filepath.extension() != ".vxproject")
		{
			VX_CONSOLE_LOG_WARN("Could not load {} - not a project file", filepath.filename().string());
			return false;
		}

		const bool projectLoaded = (bool)Project::Load(filepath);
		if (!projectLoaded)
			return false;
		
		std::string timerName = fmt::format("{} Project Load Time", filepath.filename().string());
		InstrumentationTimer timer(timerName.c_str());

		ScriptEngine::Init();
		TagComponent::ResetAddedMarkers();

		return true;
	}

	bool ProjectLoader::SaveActiveEditorProject()
	{
		return Project::GetActive()->SaveToDisk();
	}

	bool ProjectLoader::LoadRuntimeProject()
	{
		VX_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

}

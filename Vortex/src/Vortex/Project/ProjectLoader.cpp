#include "vxpch.h"
#include "ProjectLoader.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Components.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	bool ProjectLoader::LoadEditorProject(const std::filesystem::path& filepath)
	{
		const std::string filename = filepath.filename().string();
		if (filepath.extension() != ".vxproject")
		{
			VX_CONSOLE_LOG_WARN("Failed to load project '{}' - not a vortex project file!", filename);
			return false;
		}

		const std::string timerName = fmt::format("{} Project Load Time", filename);
		InstrumentationTimer timer(timerName.c_str());

		const bool projectLoaded = (bool)Project::Load(filepath);
		if (!projectLoaded)
		{
			return false;
		}

		ScriptEngine::Init();
		TagComponent::ResetAddedMarkers();

		return true;
	}

	bool ProjectLoader::SaveActiveEditorProject()
	{
		VX_CORE_ASSERT(Project::GetActive(), "No active project!");

		SharedReference<Project> project = Project::GetActive();
		return project->SaveToDisk();
	}

	bool ProjectLoader::LoadRuntimeProject()
	{
		VX_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

}

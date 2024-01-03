#include "vxpch.h"
#include "ProjectLoader.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Components.h"

#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

	bool ProjectLoader::LoadEditorProject(const Fs::Path& filepath)
	{
		const std::string filename = filepath.filename().string();
		if (filepath.extension() != ".vxproject")
		{
			VX_CONSOLE_LOG_ERROR("[Editor] Failed to load project '{}' - not a vortex project file!", filename);
			return false;
		}

		const std::string timerName = fmt::format("{} Project Load Time", filename);
		InstrumentationTimer timer(timerName.c_str());

		if (!Project::Load(filepath))
		{
			VX_CONSOLE_LOG_ERROR("[Editor] Failed to load project: '{}'", filepath.string());
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

		if (!project->SaveToDisk())
		{
			VX_CONSOLE_LOG_ERROR("[Editor] Failed to save project '{}'", project->GetName());
			return false;
		}

		return true;
	}

	bool ProjectLoader::LoadRuntimeProject()
	{
		VX_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

}

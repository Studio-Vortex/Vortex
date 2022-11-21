#include "sppch.h"
#include "Project.h"

#include "Sparky/Project/ProjectSerializer.h"

namespace Sparky {

	SharedRef<Project> Project::New()
	{
		s_ActiveProject = CreateShared<Project>();
		return s_ActiveProject;
	}

	SharedRef<Project> Project::Load(const std::filesystem::path& path)
	{
		SharedRef<Project> project = CreateShared<Project>();

		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->m_ProjectDirectory = path.parent_path(); 
			s_ActiveProject = project;
			return s_ActiveProject;
		}
		
		return project;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path(); 
			return true;
		}

		return false;
	}

}

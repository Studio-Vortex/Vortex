#include "vxpch.h"
#include "Project.h"

#include "Vortex/Project/ProjectSerializer.h"

namespace Vortex {

	SharedRef<Project> Project::New()
	{
		s_ActiveProject = SharedRef<Project>::Create();
		return s_ActiveProject;
	}

	SharedRef<Project> Project::Load(const std::filesystem::path& path)
	{
		Project::New();

		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Deserialize(path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path(); 
			return s_ActiveProject;
		}
		
		return s_ActiveProject;
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

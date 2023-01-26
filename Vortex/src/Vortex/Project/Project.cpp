#include "vxpch.h"
#include "Project.h"

#include "Vortex/Project/ProjectSerializer.h"
#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	SharedRef<Project> Project::New()
	{
		s_ActiveProject = SharedRef<Project>::Create();
		return s_ActiveProject;
	}

	SharedRef<Project> Project::Load(const std::filesystem::path& filepath)
	{
		Project::New();

		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Deserialize(filepath))
		{
			s_ActiveProject->m_ProjectDirectory = FileSystem::GetParentDirectory(filepath);
			s_AssetManager = SharedRef<EditorAssetManager>::Create();
			return s_ActiveProject;
		}
		
		return s_ActiveProject;
	}

	SharedRef<Project> Project::LoadRuntime(const std::filesystem::path& filepath)
	{
		return Project::New();
	}

	bool Project::SaveActive(const std::filesystem::path& filepath)
	{
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(filepath))
		{
			s_ActiveProject->m_ProjectDirectory = FileSystem::GetParentDirectory(filepath);
			return true;
		}

		return false;
	}

}

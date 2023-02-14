#include "vxpch.h"
#include "Project.h"

#include "Vortex/Project/ProjectSerializer.h"
#include "Vortex/Utils/FileSystem.h"
#include "Vortex/Renderer/MeshFactory.h"

namespace Vortex {

	SharedRef<Project> Project::New()
	{
		s_ActiveProject = CreateShared<Project>();
		return s_ActiveProject;
	}

	SharedRef<Project> Project::Load(const std::filesystem::path& filepath)
	{
		Project::New();

		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Deserialize(filepath))
		{
			s_ActiveProject->m_ProjectDirectory = FileSystem::GetParentDirectory(filepath);
			s_ActiveProject->m_ProjectFilepath = filepath;
			s_AssetManager = CreateShared<EditorAssetManager>();
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
			s_ActiveProject->m_ProjectFilepath = filepath;
			return true;
		}

		return false;
	}

}

#include "vxpch.h"
#include "Project.h"

#include "Vortex/Project/ProjectSerializer.h"
#include "Vortex/Utils/FileSystem.h"

namespace Vortex {
    
    SharedReference<IAssetManager> Project::GetAssetManager()
    {
        return s_AssetManager;
    }

    SharedReference<EditorAssetManager> Project::GetEditorAssetManager()
    {
        return s_AssetManager.As<EditorAssetManager>();
    }

	SharedReference<RuntimeAssetManager> Project::GetRuntimeAssetManager()
	{
		return s_AssetManager.As<RuntimeAssetManager>();
	}

    SharedRef<Project> Project::New()
	{
		if (s_ActiveProject)
		{
			s_ActiveProject = nullptr;
			s_AssetManager = nullptr;
		}

		s_ActiveProject = CreateShared<Project>();
		return s_ActiveProject;
	}

	SharedRef<Project> Project::Load(const std::filesystem::path& filepath)
	{
		VX_PROFILE_FUNCTION();

		Project::New();

		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Deserialize(filepath))
		{
			s_ActiveProject->m_ProjectDirectory = FileSystem::GetParentDirectory(filepath);
			s_ActiveProject->m_ProjectFilepath = filepath;
			s_AssetManager = SharedReference<EditorAssetManager>::Create();

			s_ActiveProject->OnDeserialized();

			return s_ActiveProject;
		}
		
		return s_ActiveProject;
	}

	SharedRef<Project> Project::LoadRuntime(const std::filesystem::path& filepath)
	{
		VX_PROFILE_FUNCTION();

		Project::New();

		/*ProjectSerializer serializer(s_ActiveProject);
		if (serializer.DeserializeRuntime())
		{
			s_AssetManager = SharedReference<RuntimeAssetManager>::Create();
		}*/

		return s_ActiveProject;
	}

    void Project::SubmitSceneToBuild(const std::string& filepath)
    {
		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

		BuildIndexMap& buildIndices = GetScenesInBuild();

		const uint32_t buildIndex = (uint32_t)buildIndices.size();
		buildIndices[buildIndex] = filepath;
    }

    BuildIndexMap& Project::GetScenesInBuild()
    {
		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

        return s_ActiveProject->m_Properties.BuildProps.BuildIndices;
    }

	bool Project::SaveToDisk()
	{
		return OnSerialized();
	}

	bool Project::OnSerialized()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

		ProjectSerializer serializer(s_ActiveProject);
		const auto& projectPath = GetProjectFilepath();
		const bool serialized = serializer.Serialize(projectPath);

		if (!serialized)
			return false;

		s_ActiveProject->m_ProjectDirectory = FileSystem::GetParentDirectory(projectPath);
		s_ActiveProject->m_ProjectFilepath = projectPath;

		return s_AssetManager.As<EditorAssetManager>()->OnSerialized();
	}

	bool Project::OnDeserialized()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

		return s_AssetManager.As<EditorAssetManager>()->OnDeserialized();
	}

}

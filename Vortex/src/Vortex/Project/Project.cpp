#include "vxpch.h"
#include "Project.h"

#include "Vortex/Project/ProjectSerializer.h"

namespace Vortex {

	ProjectProperties& Project::GetProperties()
	{
		return m_Properties;
	}
	
	const ProjectProperties& Project::GetProperties() const
	{
		return m_Properties;
	}
	
	const std::string& Project::GetName() const
	{
		return m_Properties.General.Name;
	}
	
	SharedReference<Project> Project::GetActive()
	{
		return s_ActiveProject;
	}

	const Fs::Path& Project::GetProjectDirectory()
	{
		VX_CORE_ASSERT(s_ActiveProject, "No active project!");
		return s_ActiveProject->m_ProjectDirectory;
	}

	const Fs::Path& Project::GetProjectFilepath()
	{
		VX_CORE_ASSERT(s_ActiveProject, "No active project!");
		return s_ActiveProject->m_ProjectFilepath;
	}

	Fs::Path Project::GetAssetDirectory()
	{
		VX_CORE_ASSERT(s_ActiveProject, "No active project!");
		return GetProjectDirectory() / s_ActiveProject->m_Properties.General.AssetDirectory;
	}

	Fs::Path Project::GetAssetRegistryPath()
	{
		VX_CORE_ASSERT(s_ActiveProject, "No active Project!");
		return GetAssetDirectory() / s_ActiveProject->m_Properties.General.AssetRegistryPath;
	}

	Fs::Path Project::GetCacheDirectory()
	{
		VX_CORE_ASSERT(s_ActiveProject, "No active project!");
		return GetProjectDirectory() / "Cache";
	}

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

    SharedReference<Project> Project::New()
	{
		if (s_ActiveProject)
		{
			s_ActiveProject = nullptr;
			s_AssetManager = nullptr;
		}

		s_ActiveProject = SharedReference<Project>::Create();
		return s_ActiveProject;
	}

	SharedReference<Project> Project::Load(const Fs::Path& filepath)
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
		}
		
		return s_ActiveProject;
	}

	SharedReference<Project> Project::LoadRuntime(const Fs::Path& filepath)
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

	bool Project::SaveToDisk()
	{
		return OnSerialized();
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

		return s_AssetManager.As<EditorAssetManager>()->OnProjectSerialized();
	}

	bool Project::OnDeserialized()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

		return s_AssetManager.As<EditorAssetManager>()->OnProjectDeserialized();
	}

}

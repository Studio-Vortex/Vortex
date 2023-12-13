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
		return GetAssetDirectory() / "Cache";
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

		SharedReference<Project> project = Project::New();

		ProjectSerializer serializer(project);
		bool success = serializer.Deserialize(filepath);
		if (!success)
		{
			return nullptr;
		}

		const Fs::Path directory = FileSystem::GetParentDirectory(filepath);

		project->m_ProjectDirectory = directory;
		project->m_ProjectFilepath = filepath;
		project->s_AssetManager = SharedReference<EditorAssetManager>::Create();
		
		success = project->OnDeserialized();

		if (!success)
		{
			return nullptr;
		}

		return project;
	}

	SharedReference<Project> Project::LoadRuntime(const Fs::Path& filepath)
	{
		VX_PROFILE_FUNCTION();

		Project::New();

		//ProjectSerializer serializer(s_ActiveProject);
		//if (serializer.DeserializeRuntime())
		{
			//s_AssetManager = SharedReference<RuntimeAssetManager>::Create();
		}

		return s_ActiveProject;
	}

	bool Project::SaveToDisk()
	{
		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

		return OnSerialized();
	}

	bool Project::OnSerialized()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

		SharedReference<Project> project = GetActive();

		ProjectSerializer serializer(project);
		const Fs::Path& projectFilepath = GetProjectFilepath();
		const bool success = serializer.Serialize(projectFilepath);

		if (!success)
		{
			return false;
		}

		const Fs::Path directory = FileSystem::GetParentDirectory(projectFilepath);

		project->m_ProjectDirectory = directory;
		project->m_ProjectFilepath = projectFilepath;

		return GetEditorAssetManager()->OnProjectSerialized();
	}

	bool Project::OnDeserialized()
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(s_ActiveProject, "No Active Project!");

		return GetEditorAssetManager()->OnProjectDeserialized();
	}

}

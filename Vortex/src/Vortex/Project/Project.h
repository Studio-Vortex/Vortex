#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/ReferenceCounting/RefCounted.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Asset/AssetManager/IAssetManager.h"
#include "Vortex/Asset/AssetManager/EditorAssetManager.h"
#include "Vortex/Asset/AssetManager/RuntimeAssetManager.h"

#include "Vortex/Project/ProjectProperties.h"

#include "Vortex/Utils/FileSystem.h"

namespace Vortex {

	class VORTEX_API Project : public RefCounted
	{
	public:
		ProjectProperties& GetProperties();
		const ProjectProperties& GetProperties() const;
		const std::string& GetName() const;

		static SharedReference<Project> GetActive();

		static const Fs::Path& GetProjectDirectory();
		static const Fs::Path& GetProjectFilepath();
		static Fs::Path GetAssetDirectory();
		static Fs::Path GetAssetRegistryPath();
		static Fs::Path GetCacheDirectory();

		static SharedReference<IAssetManager> GetAssetManager();
		static SharedReference<EditorAssetManager> GetEditorAssetManager();
		static SharedReference<RuntimeAssetManager> GetRuntimeAssetManager();

		static SharedReference<Project> New();
		static SharedReference<Project> Load(const Fs::Path& filepath);
		static SharedReference<Project> LoadRuntime(const Fs::Path& filepath);
		bool SaveToDisk();

		static void SubmitSceneToBuild(const std::string& filepath);
		static BuildIndexMap& GetScenesInBuild();
		
	private:
		bool OnSerialized();
		bool OnDeserialized();

	private:
		ProjectProperties m_Properties;
		Fs::Path m_ProjectDirectory = "";
		Fs::Path m_ProjectFilepath = "";

		inline static SharedReference<IAssetManager> s_AssetManager = nullptr;
		inline static SharedReference<Project> s_ActiveProject = nullptr;
	};

}

#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/ReferenceCounting/RefCounted.h"
#include "Vortex/Core/ReferenceCounting/SharedRef.h"

#include "Vortex/Asset/AssetManager/IAssetManager.h"
#include "Vortex/Asset/AssetManager/EditorAssetManager.h"
#include "Vortex/Asset/AssetManager/RuntimeAssetManager.h"

#include "Vortex/Project/ProjectProperties.h"

namespace Vortex {

	class VORTEX_API Project : public RefCounted
	{
	public:
		ProjectProperties& GetProperties();
		const ProjectProperties& GetProperties() const;
		const std::string& GetName() const;

		static SharedReference<Project> GetActive();

		static const std::filesystem::path& GetProjectDirectory();
		static const std::filesystem::path& GetProjectFilepath();
		static std::filesystem::path GetAssetDirectory();
		static std::filesystem::path GetAssetRegistryPath();
		static std::filesystem::path GetCacheDirectory();

		static SharedReference<IAssetManager> GetAssetManager();
		static SharedReference<EditorAssetManager> GetEditorAssetManager();
		static SharedReference<RuntimeAssetManager> GetRuntimeAssetManager();

		static SharedReference<Project> New();
		static SharedReference<Project> Load(const std::filesystem::path& filepath);
		static SharedReference<Project> LoadRuntime(const std::filesystem::path& filepath);
		bool SaveToDisk();

		static void SubmitSceneToBuild(const std::string& filepath);
		static BuildIndexMap& GetScenesInBuild();
		
	private:
		bool OnSerialized();
		bool OnDeserialized();

	private:
		ProjectProperties m_Properties;
		std::filesystem::path m_ProjectDirectory = "";
		std::filesystem::path m_ProjectFilepath = "";

		inline static SharedReference<IAssetManager> s_AssetManager = nullptr;
		inline static SharedReference<Project> s_ActiveProject = nullptr;
	};

}

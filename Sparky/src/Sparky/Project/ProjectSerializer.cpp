#include "sppch.h"
#include "ProjectSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Sparky {

	ProjectSerializer::ProjectSerializer(const SharedRef<Project>& project)
		: m_Project(project) { }

	void ProjectSerializer::Serialize(const std::string& filepath)
	{
		const ProjectProperties& props = m_Project->GetProperties();

		YAML::Emitter out;
		out << YAML::BeginMap;
	}

	void ProjectSerializer::SerializeRuntime(const std::string& filepath)
	{
		SP_CORE_ASSERT(false, "Not Implemented Yet!");
	}

	bool ProjectSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException& e)
		{
			SP_CORE_ERROR("Failed to load .sproject file '{}'\n     {}", filepath, e.what());
			return false;
		}

		return true;
	}

	bool ProjectSerializer::DeserializeRuntime(const std::string& filepath)
	{
		SP_CORE_ASSERT(false, "Not Implemented Yet!");
		return false;
	}

}

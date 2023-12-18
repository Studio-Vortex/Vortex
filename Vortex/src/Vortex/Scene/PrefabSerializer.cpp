#include "vxpch.h"
#include "PrefabSerializer.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Serialization/SceneSerializer.h"

#include "Vortex/Utils/FileSystem.h"

#include <yaml-cpp/yaml.h>

namespace Vortex {

	PrefabSerializer::PrefabSerializer(const SharedRef<Prefab>& prefab)
		: m_Prefab(prefab) { }

	bool PrefabSerializer::Serialize(const std::string& filepath)
	{
		SharedReference<Scene> scene = m_Prefab->m_Scene;

		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Prefab";
		out << YAML::Value << YAML::BeginSeq;

		scene->m_Registry.each([&](auto& entityID)
		{
			Actor entity{ entityID, scene.Raw() };

			if (!entity || !entity.HasComponent<IDComponent>())
			{
				return false;
			}

			SceneSerializer::SerializeActor(out, entity);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
		return true;
	}

	bool PrefabSerializer::Deserialize(const std::string& filepath, PrefabComponent& prefabComponent)
	{
		std::ifstream stream(filepath);
		if (!stream.is_open())
		{
			return false;
		}

		std::stringstream stringStream;
		stringStream << stream.rdbuf();

		YAML::Node data = YAML::Load(stringStream.str());
		if (!data["Prefab"])
			return false;

		YAML::Node prefabNode = data["Prefab"];
		SharedRef<Prefab> prefab = Prefab::Create(FileSystem::Relative(filepath, Project::GetProjectDirectory()));
		SceneSerializer::DeserializeActors(prefabNode, prefab->m_Scene);

		return true;
	}

}

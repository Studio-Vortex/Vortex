#include "vxpch.h"
#include "PrefabSerializer.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Serialization/SceneSerializer.h"

#include <yaml-cpp/yaml.h>

namespace Vortex {

	PrefabSerializer::PrefabSerializer(const SharedRef<Prefab>& prefab)
		: m_Prefab(prefab) { }

	bool PrefabSerializer::Serialize(const std::filesystem::path& path)
	{
		SharedRef<Scene> scene = m_Prefab->m_Scene;
		YAML::Emitter out;

		out << YAML::BeginMap;
		out << YAML::Key << "Prefab";
		out << YAML::Value << YAML::BeginSeq;

		scene->m_Registry.each([&](auto& entityID)
		{
			Entity entity{ entityID, scene.get() };

			if (!entity || !entity.HasComponent<IDComponent>())
			{
				return false;
			}

			SceneSerializer::SerializeEntity(out, entity);
		});

		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
		return true;
	}

	bool PrefabSerializer::Deserialize(const std::filesystem::path& path, PrefabComponent& prefabComponent)
	{
		std::ifstream stream(path);
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
		SharedRef<Prefab> prefab = Prefab::Create(std::filesystem::relative(path, Project::GetActive()->GetProjectDirectory()));
		SceneSerializer::DeserializeEntities(prefabNode, prefab->m_Scene);
		prefabComponent.EntityPrefab = prefab;

		return true;
	}

}

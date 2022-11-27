#pragma once

#include "Sparky/Scene/Scene.h"
#include "Sparky/Core/Base.h"

namespace YAML {

	class Emitter;
	class Node;

}

namespace Sparky {

	class Entity;

	class SceneSerializer
	{
	public:
		SceneSerializer(const SharedRef<Scene>& scene);
		
		static void SerializeEntity(YAML::Emitter& out, Entity entity);
		static void DeserializeEntities(YAML::Node& entitiesNode, const SharedRef<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

	private:
		SharedRef<Scene> m_Scene = nullptr;
	};

}

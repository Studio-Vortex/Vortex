#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Scene/Scene.h"

#include <string>

namespace YAML {

	class Emitter;
	class Node;

}

namespace Vortex {

	class Actor;

	class SceneSerializer
	{
	public:
		SceneSerializer() = default;
		SceneSerializer(const SharedReference<Scene>& scene);
		
		static void SerializeActor(YAML::Emitter& out, Actor actor);
		static void DeserializeActors(YAML::Node& actorsNode, SharedReference<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

	private:
		SharedReference<Scene> m_Scene = nullptr;
	};

}

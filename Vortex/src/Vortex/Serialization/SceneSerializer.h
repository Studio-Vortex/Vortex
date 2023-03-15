#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Asset/AssetSerializer.h"

namespace YAML {

	class Emitter;
	class Node;

}

namespace Vortex {

	class Entity;

	class SceneSerializer : public AssetSerializer
	{
	public:
		SceneSerializer(const SharedRef<Scene>& scene);
		
		static void SerializeEntity(YAML::Emitter& out, Entity entity);
		static void DeserializeEntities(YAML::Node& entitiesNode, SharedRef<Scene>& scene);

		void Serialize(const std::string& filepath);
		void SerializeRuntime(const std::string& filepath);

		bool Deserialize(const std::string& filepath);
		bool DeserializeRuntime(const std::string& filepath);

		void Serialize(const AssetMetadata& metadata, const SharedRef<Asset>& asset) override;
		bool TryLoadData(const AssetMetadata& metadata, SharedRef<Asset> asset) override;

	private:
		SharedRef<Scene> m_Scene = nullptr;
	};

}

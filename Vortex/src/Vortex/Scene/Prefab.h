#pragma once

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Asset/Asset.h"

#include <filesystem>

namespace Vortex {

	class Prefab : public Asset
	{
	public:
		Prefab(const std::filesystem::path& filepath);
		Prefab(Entity entity);
		~Prefab() override = default;

		static AssetType GetStaticType() { return AssetType::Prefab; }
		AssetType GetAssetType() const override { return AssetType::Prefab; }

		// Create a prefab with an empty entity
		static SharedRef<Prefab> Create(const std::filesystem::path& filepath);
		// Replaces existing entity if present
		static SharedRef<Prefab> Create(Entity entity);

	private:
		Entity CreatePrefabFromEntity(Entity entity);

	private:
		std::filesystem::path m_Filepath;
		SharedRef<Scene> m_Scene = nullptr;
		Entity m_Entity;

		friend class Scene;
		friend class ScriptEngine;
		friend class PrefabSerializer;
	};

}

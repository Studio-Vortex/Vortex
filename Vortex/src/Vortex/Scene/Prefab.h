#pragma once

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"

#include <filesystem>

namespace Vortex {

	class Prefab
	{
	public:
		Prefab(const std::filesystem::path& filepath);
		Prefab(Entity entity);
		~Prefab() = default;

		// Create a prefab with an empty entity
		static SharedRef<Prefab> Create(const std::filesystem::path& filepath);
		// Replaces existing entity if present
		static SharedRef<Prefab> Create(Entity entity);

	private:
		Entity CreatePrefabFromEntity(Entity entity);

	private:
		std::filesystem::path m_Filepath;
		SharedReference<Scene> m_Scene = nullptr;
		Entity m_Entity;

		friend class Scene;
		friend class ScriptEngine;
		friend class PrefabSerializer;
	};

}

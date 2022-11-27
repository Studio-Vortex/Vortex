#pragma once

#include "Sparky/Scene/Scene.h"
#include "Sparky/Scene/Entity.h"

#include <filesystem>

namespace Sparky {

	class Prefab
	{
	public:
		Prefab(const std::filesystem::path& filepath);
		Prefab(Entity entity);
		~Prefab();

		// Create a prefab with an empty entity
		static SharedRef<Prefab> Create(const std::filesystem::path& filepath);
		// Replaces existing entity if present
		static SharedRef<Prefab> Create(Entity entity);

	private:
		Entity CreatePrefabFromEntity(Entity entity, bool serialize = true);

	private:
		std::filesystem::path m_Filepath;
		SharedRef<Scene> m_Scene = nullptr;
		Entity m_Entity;

		friend class Scene;
		friend class ScriptEngine;
		friend class PrefabSerializer;
	};

}
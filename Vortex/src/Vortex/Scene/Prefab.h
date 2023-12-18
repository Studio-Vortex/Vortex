#pragma once

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"

#include <filesystem>

namespace Vortex {

	class Prefab
	{
	public:
		Prefab(const std::filesystem::path& filepath);
		Prefab(Actor actor);
		~Prefab() = default;

		// Create a prefab with an empty actor
		static SharedRef<Prefab> Create(const std::filesystem::path& filepath);
		// Replaces existing actor if present
		static SharedRef<Prefab> Create(Actor actor);

	private:
		Actor CreatePrefabFromActor(Actor actor);

	private:
		std::filesystem::path m_Filepath;
		SharedReference<Scene> m_Scene = nullptr;
		Actor m_Actor;

		friend class Scene;
		friend class ScriptEngine;
		friend class PrefabSerializer;
	};

}

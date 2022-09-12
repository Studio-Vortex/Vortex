#pragma once

#include "Sparky/Core/TimeStep.h"

#include <entt/entt.hpp>

namespace Sparky {

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());

		void OnUpdate(TimeStep delta);

	private:
		entt::registry m_Registry;

		friend class Entity;
	};

}

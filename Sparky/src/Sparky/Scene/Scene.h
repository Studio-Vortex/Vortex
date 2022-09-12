#pragma once

#include "Sparky/Core/TimeStep.h"

#include <entt/entt.hpp>

namespace Sparky {

	class Scene
	{
	public:
		Scene();
		~Scene();

		entt::entity CreateEntity();

		// TEMP
		entt::registry& Fuck() { return m_Registry; }

		void OnUpdate(TimeStep delta);

	private:
		entt::registry m_Registry;
	};

}

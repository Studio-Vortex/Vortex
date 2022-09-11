#pragma once

#include <entt/entt.hpp>

namespace Sparky {

	class Scene
	{
	public:
		Scene();
		~Scene();

	private:
		entt::registry m_Registry;
	};

}

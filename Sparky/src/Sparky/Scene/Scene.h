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

		void OnViewportResize(uint32_t width, uint32_t height);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

	private:
		friend class Entity;
		friend class SceneHierarchyPanel;
	};

}

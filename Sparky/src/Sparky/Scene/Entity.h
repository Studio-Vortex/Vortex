#pragma once

#include "Scene.h"

#include <entt/entt.hpp>

namespace Sparky {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity&) = default;
		
		template <typename TComponent, typename... Args>
		inline TComponent& AddComponent(Args&&... args)
		{
			SP_CORE_ASSERT(!HasComponent<TComponent>(), "Entity already has this Component!");

			return m_Scene->m_Registry.emplace<TComponent>(m_EntityID, std::forward<Args>(args)...);
		}

		template <typename... TComponents>
		inline void RemoveComponent()
		{
			SP_CORE_ASSERT(HasComponent<TComponents...>(), "Entity does not have this Component!");

			m_Scene->m_Registry.remove<TComponents...>(m_EntityID);
		}

		template <typename TComponent>
		inline TComponent& GetComponent()
		{
			SP_CORE_ASSERT(HasComponent<TComponent>(), "Entity does not have this Component!");

			return m_Scene->m_Registry.get<TComponent>(m_EntityID);
		}

		template <typename... TComponents>
		inline bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<TComponents...>(m_EntityID);
		}

		operator bool() const { return m_EntityID != entt::null; }

	private:
		entt::entity m_EntityID = entt::null;
		Scene* m_Scene = nullptr;
	};

}

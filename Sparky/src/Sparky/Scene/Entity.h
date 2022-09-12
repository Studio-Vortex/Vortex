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
		
		template <typename T, typename... Args>
		inline T& AddComponent(Args&&... args)
		{
			SP_CORE_ASSERT(!HasComponent<T>(), "Entity already has this Component!");

			return m_Scene->m_Registry.emplace<T>(m_EntityID, std::forward<Args>(args)...);
		}

		template <typename... T>
		inline void RemoveComponent()
		{
			SP_CORE_ASSERT(HasComponent<T...>(), "Entity does not have this Component!");

			m_Scene->m_Registry.remove<T...>(m_EntityID);
		}

		template <typename T>
		inline T& GetComponent()
		{
			SP_CORE_ASSERT(HasComponent<T>(), "Entity does not have this Component!");

			return m_Scene->m_Registry.get<T>(m_EntityID);
		}

		template <typename... T>
		inline bool HasComponent()
		{
			return m_Scene->m_Registry.all_of<T...>(m_EntityID);
		}

		operator bool() const { return m_EntityID != entt::null; }

	private:
		entt::entity m_EntityID = entt::null;
		Scene* m_Scene = nullptr;
	};

}

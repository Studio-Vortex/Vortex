#pragma once

#include "Vortex/Core/Base.h"
#include "Vortex/Core/UUID.h"
#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Components.h"

#include <entt/entt.hpp>

namespace Vortex {

	class Entity
	{
	public:
		Entity() = default;
		Entity(const Entity&) = default;
		
		Entity(entt::entity handle, Scene* scene)
			: m_EntityID(handle), m_Scene(scene) { }

		~Entity() = default;

		template <typename TComponent, typename... Args>
		inline TComponent& AddComponent(Args&&... args) const
		{
			VX_CORE_ASSERT(!HasComponent<TComponent>(), "Entity already has this Component!");
			TComponent& component = m_Scene->m_Registry.emplace<TComponent>(m_EntityID, std::forward<Args>(args)...);
			return component;
		}

		template <typename TComponent, typename... Args>
		inline TComponent& AddOrReplaceComponent(Args&&... args) const
		{
			TComponent& component = m_Scene->m_Registry.emplace_or_replace<TComponent>(m_EntityID, std::forward<Args>(args)...);
			return component;
		}

		template <typename... TComponent>
		inline void RemoveComponent() const
		{
			VX_CORE_ASSERT(HasComponent<TComponent...>(), "Entity does not have this Component!");
			m_Scene->m_Registry.remove<TComponent...>(m_EntityID);
		}

		template <typename TComponent>
		inline TComponent& GetComponent() const
		{
			VX_CORE_ASSERT(HasComponent<TComponent>(), "Entity does not have this Component!");
			return m_Scene->m_Registry.get<TComponent>(m_EntityID);
		}

		template <typename... TComponent>
		inline bool HasComponent() const
		{
			return m_Scene->m_Registry.all_of<TComponent...>(m_EntityID);
		}

		template <typename... TComponent>
		inline bool HasAny() const
		{
			return m_Scene->m_Registry.any_of<TComponent...>(m_EntityID);
		}

		inline UUID GetUUID() const { return GetComponent<IDComponent>().ID; }

		inline Scene* GetContextScene() { return m_Scene; }
		inline const Scene* GetContextScene() const { return m_Scene; }
		
		inline const std::string& GetName() const { return GetComponent<TagComponent>().Tag; }
		inline const std::string& GetMarker() const { return GetComponent<TagComponent>().Marker; }
		
		inline void SetTransform(const Math::mat4& transform) { GetComponent<TransformComponent>().SetTransform(transform); }
		inline TransformComponent& GetTransform() { return GetComponent<TransformComponent>(); }

		inline bool IsActive() const { return GetComponent<TagComponent>().IsActive; }
		void SetActive(bool active);

		inline Entity GetParent() const { return m_Scene->TryGetEntityWithUUID(GetParentUUID()); }

		inline UUID GetParentUUID() const { return GetComponent<HierarchyComponent>().ParentUUID; }
		inline void SetParentUUID(UUID parentUUID) { GetComponent<HierarchyComponent>().ParentUUID = parentUUID; }

		inline std::vector<UUID>& Children() { return GetComponent<HierarchyComponent>().Children; }
		inline const std::vector<UUID>& Children() const { return GetComponent<HierarchyComponent>().Children; }
		inline void AddChild(UUID childUUID) { GetComponent<HierarchyComponent>().Children.push_back(childUUID); }
		inline void RemoveChild(UUID childUUID)
		{
			auto& children = GetComponent<HierarchyComponent>().Children;
			auto it = std::find(children.begin(), children.end(), childUUID);
			VX_CORE_ASSERT(it != children.end(), "Child UUID was not found");
			children.erase(it);
		}

		inline bool HasParent() { return GetParentUUID() != 0; }

		inline bool IsAncesterOf(Entity entity)
		{
			const auto& children = GetComponent<HierarchyComponent>().Children;

			if (children.size() == 0)
				return false;

			for (UUID child : children)
			{
				if (child == entity.GetUUID())
					return true;
			}

			for (UUID child : children)
			{
				if (m_Scene->TryGetEntityWithUUID(child).IsAncesterOf(entity))
					return true;
			}

			return false;
		}

		inline bool IsDescendantOf(Entity entity)
		{
			return entity.IsAncesterOf(*this);
		}

		inline bool operator==(const Entity& other) const
		{
			return m_EntityID == other.m_EntityID && m_Scene == other.m_Scene;
		}

		inline bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		inline operator bool() const { return m_EntityID != entt::null; }
		inline operator UUID() const { return GetUUID(); }
		inline operator uint32_t() const { return (uint32_t)m_EntityID; }
		inline operator entt::entity() const { return m_EntityID; }

	private:
		entt::entity m_EntityID = entt::null;
		Scene* m_Scene = nullptr;

	private:
		friend class Prefab;
	};

}

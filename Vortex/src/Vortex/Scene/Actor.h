#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Scripting/ManagedMethods.h"

#include <entt/entt.hpp>

namespace Vortex {

	class VORTEX_API Actor
	{
	public:
		Actor() = default;
		Actor(const Actor&) = default;
		
		Actor(entt::entity handle, Scene* scene)
			: m_ActorID(handle), m_Scene(scene) { }

		~Actor() = default;

		template <typename TComponent, typename... Args>
		VX_FORCE_INLINE TComponent& AddComponent(Args&&... args) const
		{
			VX_CORE_ASSERT(!HasComponent<TComponent>(), "Actor already has this Component!");
			TComponent& component = m_Scene->m_Registry.emplace<TComponent>(m_ActorID, std::forward<Args>(args)...);
			return component;
		}

		template <typename TComponent, typename... Args>
		VX_FORCE_INLINE TComponent& AddOrReplaceComponent(Args&&... args) const
		{
			TComponent& component = m_Scene->m_Registry.emplace_or_replace<TComponent>(m_ActorID, std::forward<Args>(args)...);
			return component;
		}

		template <typename... TComponent>
		VX_FORCE_INLINE void RemoveComponent() const
		{
			VX_CORE_ASSERT(HasComponent<TComponent...>(), "Actor does not have this Component!");
			m_Scene->m_Registry.remove<TComponent...>(m_ActorID);
		}

		template <typename TComponent>
		VX_FORCE_INLINE TComponent& GetComponent() const
		{
			VX_CORE_ASSERT(HasComponent<TComponent>(), "Actor does not have this Component!");
			return m_Scene->m_Registry.get<TComponent>(m_ActorID);
		}

		template <typename... TComponent>
		VX_FORCE_INLINE bool HasComponent() const
		{
			return m_Scene->m_Registry.all_of<TComponent...>(m_ActorID);
		}

		template <typename... TComponent>
		VX_FORCE_INLINE bool HasAny() const
		{
			return m_Scene->m_Registry.any_of<TComponent...>(m_ActorID);
		}

		VX_FORCE_INLINE UUID GetUUID() const { return GetComponent<IDComponent>().ID; }

		VX_FORCE_INLINE Scene* GetContextScene() { return m_Scene; }
		VX_FORCE_INLINE const Scene* GetContextScene() const { return m_Scene; }
		
		VX_FORCE_INLINE const std::string& GetName() const { return GetComponent<TagComponent>().Tag; }
		VX_FORCE_INLINE const std::string& GetMarker() const { return GetComponent<TagComponent>().Marker; }
		
		VX_FORCE_INLINE void SetTransform(const Math::mat4& transform) { GetComponent<TransformComponent>().SetTransform(transform); }
		VX_FORCE_INLINE TransformComponent& GetTransform() { return GetComponent<TransformComponent>(); }

		VX_FORCE_INLINE bool IsActive() const { return GetComponent<TagComponent>().IsActive; }
		void SetActive(bool active);

		VX_FORCE_INLINE Actor GetParent() const { return m_Scene->TryGetActorWithUUID(GetParentUUID()); }

		VX_FORCE_INLINE UUID GetParentUUID() const { return GetComponent<HierarchyComponent>().ParentUUID; }
		VX_FORCE_INLINE void SetParentUUID(UUID parentUUID) { GetComponent<HierarchyComponent>().ParentUUID = parentUUID; }

		VX_FORCE_INLINE std::vector<UUID>& Children() { return GetComponent<HierarchyComponent>().Children; }
		VX_FORCE_INLINE const std::vector<UUID>& Children() const { return GetComponent<HierarchyComponent>().Children; }
		VX_FORCE_INLINE void AddChild(UUID childUUID) { GetComponent<HierarchyComponent>().Children.push_back(childUUID); }
		VX_FORCE_INLINE void RemoveChild(UUID childUUID)
		{
			auto& children = GetComponent<HierarchyComponent>().Children;
			auto it = std::find(children.begin(), children.end(), childUUID);
			VX_CORE_ASSERT(it != children.end(), "Child UUID was not found");
			children.erase(it);
		}

		VX_FORCE_INLINE bool HasParent() { return GetParentUUID() != 0; }

		VX_FORCE_INLINE bool IsAncesterOf(Actor actor)
		{
			const std::vector<UUID>& children = GetComponent<HierarchyComponent>().Children;

			if (children.size() == 0)
				return false;

			for (UUID child : children)
			{
				if (child == actor.GetUUID())
					return true;
			}

			for (UUID child : children)
			{
				if (m_Scene->TryGetActorWithUUID(child).IsAncesterOf(actor))
					return true;
			}

			return false;
		}

		VX_FORCE_INLINE bool IsDescendantOf(Actor actor)
		{
			return actor.IsAncesterOf(*this);
		}

		bool CallMethod(ManagedMethod method);

		VX_FORCE_INLINE bool operator==(const Actor& other) const
		{
			return m_ActorID == other.m_ActorID && m_Scene == other.m_Scene;
		}

		VX_FORCE_INLINE bool operator!=(const Actor& other) const
		{
			return !(*this == other);
		}

		VX_FORCE_INLINE operator bool() const { return m_ActorID != entt::null; }
		VX_FORCE_INLINE operator UUID() const { return GetUUID(); }
		VX_FORCE_INLINE operator uint32_t() const { return (uint32_t)m_ActorID; }
		VX_FORCE_INLINE operator entt::entity() const { return m_ActorID; }

	private:
		void OnEnable();
		void OnDisable();

	private:
		entt::entity m_ActorID = entt::null;
		Scene* m_Scene = nullptr;

	private:
		friend class Prefab;
	};

}

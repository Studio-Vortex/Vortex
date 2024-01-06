#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/Scripting/ManagedMethods.h"
#include "Vortex/Scripting/RuntimeMethodArgument.h"

#include <entt/entt.hpp>

#include <vector>

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
		
		VX_FORCE_INLINE void SetTransform(const Math::mat4& transform) const { GetComponent<TransformComponent>().SetTransform(transform); }
		VX_FORCE_INLINE TransformComponent& GetTransform() const { return GetComponent<TransformComponent>(); }

		VX_FORCE_INLINE bool IsActive() const { return GetComponent<TagComponent>().IsActive; }
		void SetActive(bool active) const;

		VX_FORCE_INLINE Actor GetParent() const { return m_Scene->TryGetActorWithUUID(GetParentUUID()); }

		VX_FORCE_INLINE UUID GetParentUUID() const { return GetComponent<HierarchyComponent>().ParentUUID; }
		VX_FORCE_INLINE void SetParentUUID(UUID parentUUID) const { GetComponent<HierarchyComponent>().ParentUUID = parentUUID; }

		VX_FORCE_INLINE std::vector<UUID>& Children() { return GetComponent<HierarchyComponent>().Children; }
		VX_FORCE_INLINE const std::vector<UUID>& Children() const { return GetComponent<HierarchyComponent>().Children; }
		VX_FORCE_INLINE void AddChild(UUID childUUID) const { GetComponent<HierarchyComponent>().Children.push_back(childUUID); }
		void RemoveChild(UUID childUUID) const;

		VX_FORCE_INLINE bool HasParent() const { return GetParentUUID() != 0; }

		bool IsAncesterOf(Actor actor) const;

		VX_FORCE_INLINE bool IsDescendantOf(Actor actor) const
		{
			return actor.IsAncesterOf(*this);
		}

		bool CallMethod(ManagedMethod method, const std::vector<RuntimeMethodArgument>& argumentList = {}) const;

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
		void OnEnabled() const;
		void OnDisabled() const;

	private:
		entt::entity m_ActorID = entt::null;
		Scene* m_Scene = nullptr;

	private:
		friend class Prefab;
	};

}

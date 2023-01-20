#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/TimeStep.h"
#include "Vortex/Scene/Components.h"
#include "Vortex/Editor/EditorCamera.h"

#include <entt/entt.hpp>

namespace Vortex {

	class Entity;
	class Framebuffer;

	class Scene
	{
	public:
		Scene() = default;
		Scene(const SharedRef<Framebuffer>& targetFramebuffer);
		~Scene() = default;

		static SharedRef<Scene> Copy(SharedRef<Scene> source);
		static void CreateDefaultEntities(const SharedRef<Scene>& context);

		Entity CreateEntity(const std::string& name = std::string(), const std::string& marker = std::string());
		Entity CreateChildEntity(Entity parent, const std::string& name = std::string(), const std::string& marker = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string(), const std::string& marker = std::string());
		void DestroyEntity(Entity entity, bool excludeChildren = false);

		void ParentEntity(Entity entity, Entity parent);
		void UnparentEntity(Entity entity, bool convertToWorldSpace = true);

		void ActiveateChildren(Entity entity);
		void DeactiveateChildren(Entity entity);

		void OnRuntimeStart(bool muteAudio = false);
		void OnRuntimeStop();

		void OnPhysicsSimulationStart();
		void OnPhysicsSimulationStop();

		void OnUpdateRuntime(TimeStep delta);
		void OnUpdateSimulation(TimeStep delta, EditorCamera* camera);
		void OnUpdateEditor(TimeStep delta, EditorCamera* camera);
		void OnUpdateEntityGui();

		void Step(uint32_t frames = 1);

		void OnViewportResize(uint32_t width, uint32_t height);

		inline const SharedRef<Framebuffer>& GetTargetFramebuffer() const { return m_TargetFramebuffer; }
		inline void SetTargetFramebuffer(const SharedRef<Framebuffer>& target) { m_TargetFramebuffer = target; }

		inline bool IsRunning() const { return m_IsRunning; }
		inline bool IsPaused() const { return m_IsPaused; }
		inline void SetPaused(bool paused) { m_IsPaused = paused; }

		// -------- Editor Only ---------
		inline bool IsInDebugMode() const { return m_DebugMode; }
		inline void SetDebugMode(bool mode) { m_DebugMode = mode; }
		inline void SetShouldUpdateScripts(bool shouldUpdate) { m_ShouldUpdateScripts = shouldUpdate; }
		// ------------------------------

		inline const Math::ivec2& GetViewportSize() const { return { m_ViewportWidth, m_ViewportHeight }; }
		inline size_t GetEntityCount() const { return m_Registry.alive(); }

		Entity TryGetTopEntityInHierarchy(Entity child) const;
		Entity DuplicateEntity(Entity entity);

		Entity TryGetEntityWithUUID(UUID uuid);
		Entity FindEntityByName(std::string_view name);

		Entity GetPrimaryCameraEntity();

		void SortEntities();

		void ConvertToLocalSpace(Entity entity);
		void ConvertToWorldSpace(Entity entity);
		Math::mat4 GetWorldSpaceTransformMatrix(Entity entity);
		TransformComponent GetWorldSpaceTransform(Entity entity);

		template <typename TComponent>
		inline void CopyComponentIfExists(entt::entity dst, entt::registry& dstRegistry, entt::entity src)
		{
			if (m_Registry.all_of<TComponent>(src))
			{
				auto& srcComponent = m_Registry.get<TComponent>(src);
				dstRegistry.emplace_or_replace<TComponent>(dst, srcComponent);
			}
		}

		template <typename... TComponent>
		inline auto GetAllEntitiesWith()
		{
			return m_Registry.view<TComponent...>();
		}

		static SharedRef<Scene> Create(const SharedRef<Framebuffer>& targetFramebuffer);
		static SharedRef<Scene> Create();

	private:
		template <typename TComponent>
		void OnComponentAdded(Entity entity, TComponent& component);

		void OnModelUpdate();
		void OnAnimatorUpdate(TimeStep delta);
		void OnParticleEmitterUpdate(TimeStep delta);

	private:
		SharedRef<Framebuffer> m_TargetFramebuffer = nullptr;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		uint32_t m_StepFrames = 0;

		bool m_IsRunning = false;
		bool m_IsPaused = false;

		// ------ Editor-only ------
		bool m_ShouldUpdateScripts = true;
		bool m_DebugMode = false;
		// -------------------------

		using EntityMap = std::unordered_map<UUID, Entity>;
		EntityMap m_EntityMap;

	private:
		friend class Entity;
		friend class Prefab;
		friend class PrefabSerializer;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}

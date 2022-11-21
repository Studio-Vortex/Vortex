#pragma once

#include "Sparky/Core/UUID.h"
#include "Sparky/Core/TimeStep.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Scene/SceneRenderer.h"

#include <entt/entt.hpp>

namespace Sparky {

	class Entity;

	class Scene
	{
	public:
		Scene() = default;
		~Scene() = default;

		static SharedRef<Scene> Copy(SharedRef<Scene> source);

		Entity CreateEntity(const std::string& name = std::string(), const std::string& marker = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string(), const std::string& marker = std::string());
		void DestroyEntity(Entity entity, bool isEntityInstance = false, bool excludeChildren = false);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnPhysicsSimulationStart();
		void OnPhysicsSimulationStop();

		void OnUpdateRuntime(TimeStep delta);
		void OnUpdateSimulation(TimeStep delta, EditorCamera& camera);
		void OnUpdateEditor(TimeStep delta, EditorCamera& camera);
		void OnUpdateEntityGui();

		void Step(uint32_t frames = 1);

		void OnViewportResize(uint32_t width, uint32_t height);

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }
		void SetPaused(bool paused) { m_IsPaused = paused; }
		bool IsInDebugMode() const { return m_DebugMode; }
		void SetDebugMode(bool mode) { m_DebugMode = mode; }

		size_t GetEntityCount() const { return m_Registry.alive(); }

		Entity DuplicateEntity(Entity entity);

		Entity TryGetEntityWithUUID(UUID uuid);
		Entity FindEntityByName(std::string_view name);

		Entity GetPrimaryCameraEntity();

		void ConvertToLocalSpace(Entity entity);
		void ConvertToWorldSpace(Entity entity);
		Math::mat4 GetWorldSpaceTransformMatrix(Entity entity, bool accountForPhysicsActor = false);
		TransformComponent GetWorldSpaceTransform(Entity entity);

		template <typename... TComponent>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<TComponent...>();
		}

		static SharedRef<Scene> Create();

	private:
		template <typename TComponent>
		void OnComponentAdded(Entity entity, TComponent& component);

		void OnModelUpdate();
		void OnParticleEmitterUpdate(TimeStep delta);
		void OnLightSourceUpdate();

	private:
		entt::registry m_Registry;
		SceneRenderer m_SceneRenderer;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		uint32_t m_StepFrames = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		bool m_DebugMode = false; // Editor-only

		std::unordered_map<UUID, entt::entity> m_EntityMap;

	private:
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}

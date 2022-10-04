#pragma once

#include "Sparky/Core/UUID.h"
#include "Sparky/Core/TimeStep.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Renderer/EditorCamera.h"

#include <entt/entt.hpp>

class b2World;

namespace Sparky {

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static SharedRef<Scene> Copy(SharedRef<Scene> source);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity, bool isEntityInstance = false);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(TimeStep delta);
		void OnUpdateSimulation(TimeStep delta, EditorCamera& camera);
		void OnUpdateEditor(TimeStep delta, EditorCamera& camera);

		void OnViewportResize(uint32_t width, uint32_t height);

		bool IsRunning() const { return m_IsRunning; }

		static int32_t GetPhysicsWorldVeloctiyIterations() { return s_PhysicsWorldVeloctityIterations; }
		static void SetPhysicsWorldVelocityIterations(int32_t veloctiyIterations) { s_PhysicsWorldVeloctityIterations = veloctiyIterations; }

		static int32_t GetPhysicsWorldPositionIterations() { return s_PhysicsWorldPositionIterations; }
		static void SetPhysicsWorldPositionIterations(int32_t positionIterations) { s_PhysicsWorldPositionIterations = positionIterations; }
		
		static Math::vec2 GetPhysicsWorldGravity() { return s_PhysicsWorldGravity; }
		static void SetPhysicsWorldGravitty(const Math::vec2& gravity) { s_PhysicsWorldGravity = gravity; }

		Entity DuplicateEntity(Entity entity);

		Entity GetEntityWithUUID(UUID uuid);
		Entity FindEntityByName(std::string_view name);

		Entity GetPrimaryCameraEntity();

		template <typename... TComponents>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<TComponents...>();
		}

	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);

		void CreatePhysicsBodyAndFixture(Entity entity, const TransformComponent& transform, RigidBody2DComponent& rb2d);

		void OnPhysics2DStart();
		void OnPhysics2DUpdate(TimeStep delta);
		void OnPhysics2DStop();

		void RenderScene(EditorCamera& camera);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		bool m_IsRunning = false;

		b2World* m_PhysicsWorld = nullptr;
		
		inline static Math::vec2 s_PhysicsWorldGravity = Math::vec2(0.0f, -9.8f);
		inline static int32_t s_PhysicsWorldVeloctityIterations = 6;
		inline static int32_t s_PhysicsWorldPositionIterations = 2;

		std::unordered_map<UUID, entt::entity> m_EntityMap;

	private:
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}

#pragma once

#include "Sparky/Core/UUID.h"
#include "Sparky/Core/TimeStep.h"
#include "Sparky/Scene/Components.h"
#include "Sparky/Scene/SceneRenderer.h"

#include <entt/entt.hpp>

class b2World;
class b2Fixture;

namespace Sparky {

	class Entity;

	struct PhysicsBodyData
	{
		UUID EntityUUID;
	};

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
		void OnUpdateEntityGui();

		void Step(uint32_t frames = 1);

		void OnViewportResize(uint32_t width, uint32_t height);

		bool IsRunning() const { return m_IsRunning; }
		bool IsPaused() const { return m_IsPaused; }
		void SetPaused(bool paused) { m_IsPaused = paused; }
		bool IsInDebugMode() const { return m_DebugMode; }
		void SetDebugMode(bool mode) { m_DebugMode = mode; }

		b2World* GetPhysicsWorld() { return m_PhysicsWorld; }

		static int32_t GetPhysicsWorld2DVeloctiyIterations() { return s_PhysicsWorldVeloctityIterations; }
		static void SetPhysicsWorld2DVelocityIterations(int32_t veloctiyIterations) { s_PhysicsWorldVeloctityIterations = veloctiyIterations; }

		static int32_t GetPhysicsWorld2DPositionIterations() { return s_PhysicsWorldPositionIterations; }
		static void SetPhysicsWorld2DPositionIterations(int32_t positionIterations) { s_PhysicsWorldPositionIterations = positionIterations; }
		
		static Math::vec2 GetPhysicsWorld2DGravity() { return s_PhysicsWorldGravity; }
		static void SetPhysicsWorld2DGravitty(const Math::vec2& gravity) { s_PhysicsWorldGravity = gravity; }

		Entity DuplicateEntity(Entity entity);

		Entity GetEntityWithUUID(UUID uuid);
		Entity FindEntityByName(std::string_view name);

		Entity GetPrimaryCameraEntity();

		template <typename... TComponent>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<TComponent...>();
		}

	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnCreatePhysicsBody2D(Entity entity, const TransformComponent& transform, RigidBody2DComponent& rb2d);

		void OnPhysics2DStart();
		void OnPhysics2DUpdate(TimeStep delta);
		void OnPhysics2DStop();

	private:
		entt::registry m_Registry;
		SceneRenderer m_SceneRenderer;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		uint32_t m_StepFrames = 0;
		bool m_IsRunning = false;
		bool m_IsPaused = false;
		bool m_DebugMode = false; // Editor-only

		b2World* m_PhysicsWorld = nullptr;
		
		inline static Math::vec2 s_PhysicsWorldGravity = Math::vec2(0.0f, -9.8f);
		inline static int32_t s_PhysicsWorldVeloctityIterations = 6;
		inline static int32_t s_PhysicsWorldPositionIterations = 2;

		std::unordered_map<b2Fixture*, UniqueRef<PhysicsBodyData>> m_PhysicsBodyDataMap;

		std::unordered_map<UUID, entt::entity> m_EntityMap;

	private:
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}

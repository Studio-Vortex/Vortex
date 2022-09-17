#pragma once

#include "Sparky/Core/UUID.h"
#include "Sparky/Core/TimeStep.h"
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
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(TimeStep delta);
		void OnUpdateSimulation(TimeStep delta, EditorCamera& camera);
		void OnUpdateEditor(TimeStep delta, EditorCamera& camera);

		void OnViewportResize(uint32_t width, uint32_t height);

		Entity DuplicateEntity(Entity entity);

		Entity GetPrimaryCameraEntity();

		template <typename... TComponents>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<TComponents...>();
		}

	private:
		template <typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void RenderScene(EditorCamera& camera);

	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;

		b2World* m_PhysicsWorld = nullptr;

	private:
		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}

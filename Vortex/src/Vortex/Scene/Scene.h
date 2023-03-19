#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Asset/Asset.h"
#include "Vortex/Core/TimeStep.h"
#include "Vortex/Scene/Components.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <entt/entt.hpp>

namespace Vortex {

	class Entity;
	class Mesh;
	class StaticMesh;
	class Framebuffer;
	class EditorCamera;

	class Scene : public Asset
	{
	public:
		struct QueueFreeData
		{
			UUID EntityUUID = 0;
			float WaitTime = 0.0f;
			bool ExcludeChildren = false;
		};

	public:
		Scene() = default;
		Scene(SharedRef<Framebuffer> targetFramebuffer);
		~Scene() override = default;

		Entity CreateEntity(const std::string& name = std::string(), const std::string& marker = std::string());
		Entity CreateChildEntity(Entity parent, const std::string& name = std::string(), const std::string& marker = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string(), const std::string& marker = std::string());

		void SubmitToDestroyEntity(Entity entity, bool excludeChildren = false);
		void SubmitToDestroyEntity(const QueueFreeData& queueFreeData);

	private:
		void DestroyEntity(Entity entity, bool excludeChildren = false);
		void DestroyEntity(const QueueFreeData& queueFreeData);

		void UpdateQueueFreeTimers(TimeStep delta);

	public:
		void OnRuntimeStart(bool muteAudio = false);
		void OnRuntimeStop();

		void OnPhysicsSimulationStart();
		void OnPhysicsSimulationUpdate(TimeStep delta);
		void OnPhysicsSimulationStop();

		void OnUpdateRuntime(TimeStep delta);
		void OnUpdateSimulation(TimeStep delta, EditorCamera* camera);
		void OnUpdateEditor(TimeStep delta, EditorCamera* camera);

		void SubmitToPostUpdateQueue(const std::function<void()>& func);
		void ExecutePostUpdateQueue();

		void OnUpdateEntityGui();

		inline SharedRef<Framebuffer> GetTargetFramebuffer() const { return m_TargetFramebuffer; }
		inline void SetTargetFramebuffer(SharedRef<Framebuffer> target) { m_TargetFramebuffer = target; }

		inline bool IsRunning() const { return m_IsRunning; }
		inline bool IsPaused() const { return m_IsPaused; }
		inline void SetPaused(bool paused) { m_IsPaused = paused; }

		inline const Math::ivec2& GetViewportSize() const { return { m_ViewportWidth, m_ViewportHeight }; }
		inline size_t GetEntityCount() const { return m_Registry.alive(); }

		void Step(uint32_t frames = 1) { m_StepFrames = frames; }

		void OnViewportResize(uint32_t width, uint32_t height);

		void ParentEntity(Entity entity, Entity parent);
		void UnparentEntity(Entity entity, bool convertToWorldSpace = true);

		void ActiveateChildren(Entity entity);
		void DeactiveateChildren(Entity entity);

		Entity TryGetRootEntityInHierarchy(Entity child) const;
		Entity TryGetEntityWithUUID(UUID uuid);
		Entity GetPrimaryCameraEntity();

		Entity DuplicateEntity(Entity entity);

		Entity FindEntityByName(std::string_view name);
		Entity FindEntityWithID(entt::entity entity);

		bool AreEntitiesRelated(Entity first, Entity second);

		void SortEntities();

		void ConvertToLocalSpace(Entity entity);
		void ConvertToWorldSpace(Entity entity);
		Math::mat4 GetWorldSpaceTransformMatrix(Entity entity);
		TransformComponent GetWorldSpaceTransform(Entity entity);

		struct SceneMeshes : public RefCounted
		{
			std::vector<SharedReference<Mesh>> Meshes;
			std::vector<Math::mat4> WorldSpaceMeshTransforms;
			std::vector<Entity> MeshEntities;

			std::vector<SharedReference<StaticMesh>> StaticMeshes;
			std::vector<Math::mat4> WorldSpaceStaticMeshTransforms;
		};

		SharedReference<SceneMeshes>& GetSceneMeshes();

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

#ifndef VX_DIST
		const std::string& GetDebugName() const { return m_DebugName; }
		void SetDebugName(const std::string& name) { m_DebugName = name; }
#endif

		static void SubmitSceneToBuild(const std::string& sceneFilePath);
		static void RemoveIndexFromBuild(uint32_t buildIndex);
		static const std::map<uint32_t, std::string>& GetScenesInBuild();

		static uint32_t GetActiveSceneBuildIndex();
		static void SetActiveSceneBuildIndex(uint32_t buildIndex);

		static SharedReference<Scene> Copy(SharedReference<Scene>& source);
		static void Create2DSampleScene(SharedReference<Scene>& context);
		static void Create3DSampleScene(SharedReference<Scene>& context);

		ASSET_CLASS_TYPE(SceneAsset)

		static SharedReference<Scene> Create(SharedRef<Framebuffer> targetFramebuffer);
		static SharedReference<Scene> Create();

	private:
		template <typename TComponent>
		void OnComponentAdded(Entity entity, TComponent& component);

		void CreateScriptInstancesRuntime();
		void DestroyScriptInstancesRuntime();

		void StartAudioSourcesRuntime();
		void StopAudioSourcesRuntime();

		void StopAnimatorsRuntime();

		void StopParticleEmittersRuntime();

		void OnMeshUpdateRuntime();
		void OnAnimatorUpdateRuntime(TimeStep delta);
		void OnParticleEmitterUpdateRuntime(TimeStep delta);

		void ClearSceneMeshes();

	private:
		SharedRef<Framebuffer> m_TargetFramebuffer = nullptr;
		SharedReference<SceneMeshes> m_SceneMeshes = nullptr;
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0;
		uint32_t m_ViewportHeight = 0;
		uint32_t m_StepFrames = 0;

		inline static uint32_t s_ActiveBuildIndex = 0;

		using EntityMap = std::unordered_map<UUID, Entity>;
		EntityMap m_EntityMap;

		using QueueFreeMap = std::unordered_map<UUID, QueueFreeData>;
		QueueFreeMap m_QueueFreeMap;
		std::vector<UUID> m_EntitiesToBeRemovedFromQueue;

		std::vector<std::function<void()>> m_PostUpdateQueue;
		std::mutex m_PostUpdateQueueMutex;

#ifndef VX_DIST
		std::string m_DebugName;
#endif

		bool m_IsRunning = false;
		bool m_IsPaused = false;

	private:
		friend class Entity;
		friend class Prefab;
		friend class PrefabSerializer;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
		friend class ECSDebugPanel;
		friend class EditorLayer;
	};

}

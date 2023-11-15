#pragma once

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/TimeStep.h"

#include "Vortex/Asset/Asset.h"

#include "Vortex/Scene/Components.h"

#include "Vortex/Renderer/Framebuffer.h"

#include <unordered_map>
#include <vector>

#include <entt/entt.hpp>

namespace Vortex {

	class Entity;
	class Mesh;
	class StaticMesh;
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

		struct SceneGeometry : public RefCounted
		{
			std::vector<SharedReference<Mesh>> Meshes;
			std::vector<Math::mat4> WorldSpaceMeshTransforms;
			std::vector<Entity> MeshEntities;

			std::vector<SharedReference<StaticMesh>> StaticMeshes;
			std::vector<Math::mat4> WorldSpaceStaticMeshTransforms;
		};

	public:
		Scene() = default;
		Scene(SharedReference<Framebuffer>& targetFramebuffer);
		~Scene() override;

		Entity CreateEntity(const std::string& name = std::string(), const std::string& marker = std::string());
		Entity CreateChildEntity(Entity parent, const std::string& name = std::string(), const std::string& marker = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string(), const std::string& marker = std::string());

		void SubmitToDestroyEntity(Entity entity, bool excludeChildren = false);
		void SubmitToDestroyEntity(const QueueFreeData& queueFreeData);

	private:
		void DestroyEntityInternal(Entity entity, bool excludeChildren = false);
		void DestroyEntityInternal(const QueueFreeData& queueFreeData);

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

		VX_FORCE_INLINE SharedReference<Framebuffer> GetTargetFramebuffer() const { return m_TargetFramebuffer; }
		VX_FORCE_INLINE void SetTargetFramebuffer(SharedReference<Framebuffer> target) { m_TargetFramebuffer = target; }

		VX_FORCE_INLINE bool IsRunning() const { return m_IsRunning; }
		VX_FORCE_INLINE bool IsPaused() const { return m_IsPaused; }
		void SetPaused(bool paused);

		VX_FORCE_INLINE Math::uvec2 GetViewportSize() const { return Math::uvec2(m_ViewportWidth, m_ViewportHeight); }
		VX_FORCE_INLINE size_t GetEntityCount() const { return m_Registry.alive(); }

		void Step(uint32_t frames = 1) { m_StepFrames = frames; }

		void OnViewportResize(uint32_t width, uint32_t height);

		void ParentEntity(Entity entity, Entity parent);
		void UnparentEntity(Entity entity, bool convertToWorldSpace = true);

		void ActiveateChildren(Entity entity);
		void DeactiveateChildren(Entity entity);

		Entity GetRootEntityInHierarchy(Entity child) const;
		Entity TryGetEntityWithUUID(UUID uuid);

		Entity GetPrimaryCameraEntity();
		Entity GetSkyLightEntity();

		Entity DuplicateEntity(Entity entity);

		Entity FindEntityByName(std::string_view name);
		Entity FindEntityWithID(entt::entity entity);

		bool AreEntitiesRelated(Entity first, Entity second);

		void SortEntities();

		void ConvertToLocalSpace(Entity entity);
		void ConvertToWorldSpace(Entity entity);
		Math::mat4 GetWorldSpaceTransformMatrix(Entity entity);
		TransformComponent GetWorldSpaceTransform(Entity entity);

		SharedReference<SceneGeometry>& GetSceneMeshes();

		template <typename TComponent>
		VX_FORCE_INLINE void CopyComponentIfExists(entt::entity dst, entt::registry& dstRegistry, entt::entity src)
		{
			if (m_Registry.all_of<TComponent>(src))
			{
				auto& srcComponent = m_Registry.get<TComponent>(src);
				dstRegistry.emplace_or_replace<TComponent>(dst, srcComponent);
			}
		}

		template <typename... TComponent>
		VX_FORCE_INLINE auto GetAllEntitiesWith()
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

		static SharedReference<Scene> Create(SharedReference<Framebuffer>& targetFramebuffer);
		static SharedReference<Scene> Create();

	private:
		void OnCameraConstruct(entt::registry& registry, entt::entity e);
		void OnStaticMeshConstruct(entt::registry& registry, entt::entity e);
		void OnParticleEmitterConstruct(entt::registry& registry, entt::entity e);
		void OnParticleEmitterDestruct(entt::registry& registry, entt::entity e);
		void OnTextMeshConstruct(entt::registry& registry, entt::entity e);
		void OnAnimatorConstruct(entt::registry& registry, entt::entity e);
		void OnAnimationConstruct(entt::registry& registry, entt::entity e);
		void OnAudioSourceConstruct(entt::registry& registry, entt::entity e);
		void OnAudioSourceDestruct(entt::registry& registry, entt::entity e);
		void OnAudioListenerConstruct(entt::registry& registry, entt::entity e);
		void OnAudioListenerDestruct(entt::registry& registry, entt::entity e);

		void SetSceneCameraViewportSize();

		void CreateScriptInstancesRuntime();
		void DestroyScriptInstancesRuntime();

		void StopAnimatorsRuntime();

		void OnMeshUpdateRuntime();
		void OnAnimatorUpdateRuntime(TimeStep delta);

		void ClearSceneMeshes();

	private:
		SharedReference<Framebuffer> m_TargetFramebuffer = nullptr;
		SharedReference<SceneGeometry> m_SceneMeshes = nullptr;
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

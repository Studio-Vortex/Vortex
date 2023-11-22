#include "vxpch.h"
#include "Scene.h"

#include "Vortex/Core/Math/Math.h"
#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scene/SceneRenderer.h"
#include "Vortex/Scene/ScriptableEntity.h"

#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/ParticleSystem/ParticleSystem.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

#include "Vortex/System/SystemManager.h"

#include "Vortex/Scripting/ScriptEngine.h"

#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/2D/Physics2D.h"

#include "Vortex/Editor/EditorCamera.h"
#include "Vortex/Editor/SelectionManager.h"

namespace Vortex {

	namespace Utils {

		template <typename... TComponent>
		static void CopyComponent(entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			([&]()
			{
				auto view = src.view<TComponent>();

				for (auto srcEntity : view)
				{
					entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

					const auto& srcComponent = src.get<TComponent>(srcEntity);
					dst.emplace_or_replace<TComponent>(dstEntity, srcComponent);
				}
			}(), ...);
		}

		template<typename... Component>
		static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
		{
			CopyComponent<Component...>(dst, src, enttMap);
		}

		template<typename... TComponent>
		static void CopyComponentIfExists(Entity dst, Entity src)
		{
			VX_PROFILE_FUNCTION();

			([&]()
			{
				if (src.HasComponent<TComponent>())
				{
					dst.AddOrReplaceComponent<TComponent>(src.GetComponent<TComponent>());
				}

				// NOTE: when meshes are copied over after hitting play,
				// the material values are not also set meaning the material handle wasn't set properly,
				// or the mesh could possibly have another random modified state of the material.
				// Either case this needs to be fixed.

				/*if (src.HasComponent<StaticMeshRendererComponent>())
				{
					const auto& srcMesh = src.GetComponent<StaticMeshRendererComponent>();
					auto& dstMesh = dst.GetComponent<StaticMeshRendererComponent>();

					uint32_t materialCount = srcMesh.Materials->GetMaterialCount();
					for (uint32_t i = 0; i < materialCount; i++)
					{
						AssetHandle materialHandle = srcMesh.Materials->GetMaterial(i);
						if (!AssetManager::IsHandleValid(materialHandle))
							continue;

						dstMesh.Materials->SetMaterial(i, materialHandle);
					}
				}*/
			}(), ...);
		}

		template<typename... TComponent>
		static void CopyComponentIfExists(ComponentGroup<TComponent...>, Entity dst, Entity src)
		{
			CopyComponentIfExists<TComponent...>(dst, src);
		}

	}

	static SceneRenderer s_SceneRenderer;

	Scene::Scene(SharedReference<Framebuffer>& targetFramebuffer)
		: m_TargetFramebuffer(targetFramebuffer)
	{
		SystemManager::SubmitContextScene(this);

		m_Registry.on_construct<CameraComponent>().connect<&Scene::OnCameraConstruct>(this);
		m_Registry.on_construct<StaticMeshRendererComponent>().connect<&Scene::OnStaticMeshConstruct>(this);

		m_Registry.on_construct<ParticleEmitterComponent>().connect<&Scene::OnParticleEmitterConstruct>(this);
		m_Registry.on_destroy<ParticleEmitterComponent>().connect<&Scene::OnParticleEmitterDestruct>(this);

		m_Registry.on_construct<TextMeshComponent>().connect<&Scene::OnTextMeshConstruct>(this);
		
		m_Registry.on_construct<AnimatorComponent>().connect<&Scene::OnAnimatorConstruct>(this);
		m_Registry.on_construct<AnimationComponent>().connect<&Scene::OnAnimationConstruct>(this);

		m_Registry.on_construct<AudioSourceComponent>().connect<&Scene::OnAudioSourceConstruct>(this);
		m_Registry.on_destroy<AudioSourceComponent>().connect<&Scene::OnAudioSourceDestruct>(this);
		
		m_Registry.on_construct<AudioListenerComponent>().connect<&Scene::OnAudioListenerConstruct>(this);
		m_Registry.on_destroy<AudioListenerComponent>().connect<&Scene::OnAudioListenerDestruct>(this);
	}

	Scene::~Scene()
	{
		SystemManager::RemoveContextScene();

		m_Registry.on_construct<CameraComponent>().disconnect();
		m_Registry.on_construct<StaticMeshRendererComponent>().disconnect();

		m_Registry.on_construct<ParticleEmitterComponent>().disconnect();
		m_Registry.on_destroy<ParticleEmitterComponent>().disconnect();
		
		m_Registry.on_construct<TextMeshComponent>().disconnect();
		
		m_Registry.on_construct<AnimatorComponent>().disconnect();
		m_Registry.on_construct<AnimationComponent>().disconnect();
		
		m_Registry.on_construct<AudioSourceComponent>().disconnect();
		m_Registry.on_destroy<AudioSourceComponent>().disconnect();
		
		m_Registry.on_construct<AudioListenerComponent>().disconnect();
		m_Registry.on_destroy<AudioListenerComponent>().disconnect();
	}

	Entity Scene::CreateEntity(const std::string& name, const std::string& marker)
	{
		VX_PROFILE_FUNCTION();

		return CreateEntityWithUUID(UUID(), name, marker);
	}

	Entity Scene::CreateChildEntity(Entity parent, const std::string& name, const std::string& marker)
	{
		VX_PROFILE_FUNCTION();

		Entity child = CreateEntity(name, marker);

		ParentEntity(child, parent);

		SortEntities();

		return child;
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const std::string& marker)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		tag.Marker = marker.empty() ? "Untagged" : marker;

		entity.AddComponent<HierarchyComponent>();

		// Store the entity's UUID and the entt handle in our Entity map
		// entity here will be implicitly converted to an entt handle
		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::SubmitToDestroyEntity(Entity entity, bool excludeChildren)
	{
		DestroyEntityInternal(entity, excludeChildren);

		// TODO figure out why this doesn't work
		//SubmitToPostUpdateQueue([&]() { DestroyEntityInternal(entity, excludeChildren); });
	}

	void Scene::SubmitToDestroyEntity(const QueueFreeData& queueFreeData)
	{
		DestroyEntityInternal(queueFreeData);
	}

	void Scene::DestroyEntityInternal(Entity entity, bool excludeChildren)
	{
		VX_PROFILE_FUNCTION();

		if (!entity || !m_EntityMap.contains(entity.GetUUID()))
		{
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyEntity with invalid Entity!");
			VX_CORE_ASSERT(false, "Trying to free invalid Entity!");
			return;
		}

		if (Entity selected = SelectionManager::GetSelectedEntity(); selected == entity)
		{
			SelectionManager::DeselectEntity();
		}

		if (m_IsRunning)
		{
			// Destroy script instance
			if (entity.HasComponent<ScriptComponent>() && ScriptEngine::GetContextScene() != nullptr)
			{
				const ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();

				if (ScriptEngine::EntityClassExists(scriptComponent.ClassName))
				{
					ScriptEngine::OnDestroyEntity(entity);
				}
			}

			Physics::DestroyPhysicsActor(entity);
			Physics2D::DestroyPhysicsBody(entity);
		}

		if (entity.HasParent())
		{
			if (Entity parent = entity.GetParent())
			{
				parent.RemoveChild(entity.GetUUID());
			}
		}

		if (!excludeChildren)
		{
			for (size_t i = 0; i < entity.Children().size(); i++)
			{
				auto& childID = entity.Children()[i];
				Entity child = TryGetEntityWithUUID(childID);
				DestroyEntityInternal(child, excludeChildren);
			}
		}

		auto it = m_EntityMap.find(entity.GetUUID());

		VX_CORE_ASSERT(it != m_EntityMap.end(), "Enitiy was not found in Entity Map!");

		// Remove the entity from our internal map
		m_EntityMap.erase(it->second);
		m_Registry.destroy(entity);

		SortEntities();
	}

	void Scene::DestroyEntityInternal(const QueueFreeData& queueFreeData)
	{
		VX_PROFILE_FUNCTION();

		if (!m_EntityMap.contains(queueFreeData.EntityUUID))
			return;

		if (m_QueueFreeMap.contains(queueFreeData.EntityUUID))
			return;

		const bool invalidTimer = queueFreeData.WaitTime <= 0.0f;

		if (invalidTimer)
		{
			Entity entity = m_EntityMap[queueFreeData.EntityUUID];
			SubmitToDestroyEntity(entity, queueFreeData.ExcludeChildren);
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyEntity with a wait time of 0, Use the regular method instead!");
			return;
		}

		m_QueueFreeMap[queueFreeData.EntityUUID] = queueFreeData;
	}

	void Scene::UpdateQueueFreeTimers(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		// Update timers for entites to be destroyed
		for (auto& [uuid, queueFreeData] : m_QueueFreeMap)
		{
			queueFreeData.WaitTime -= delta;

			if (queueFreeData.WaitTime <= 0.0f)
			{
				auto it = std::find(m_EntitiesToBeRemovedFromQueue.begin(), m_EntitiesToBeRemovedFromQueue.end(), uuid);
				if (it == m_EntitiesToBeRemovedFromQueue.end())
				{
					m_EntitiesToBeRemovedFromQueue.push_back(uuid);
				}
			}
		}

		for (const auto& uuid : m_EntitiesToBeRemovedFromQueue)
		{
			VX_CORE_ASSERT(m_EntityMap.contains(uuid), "Invalid Entity UUID!");
			SubmitToDestroyEntity(m_EntityMap[uuid], m_QueueFreeMap[uuid].ExcludeChildren);
			m_QueueFreeMap.erase(uuid);
		}

		m_EntitiesToBeRemovedFromQueue.clear();
	}

	void Scene::OnRuntimeStart(bool muteAudio)
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = true;

		SetSceneCameraViewportSize();

		OnPhysicsSimulationStart();

		// TODO: when we have AudioSources that play on
		// start we need to stop them here if muteAudio is true
		SystemManager::OnRuntimeStart();

		CreateScriptInstancesRuntime();
	}

	void Scene::OnRuntimeStop()
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = false;

		DestroyScriptInstancesRuntime();
		ScriptEngine::OnRuntimeStop();

		SystemManager::OnRuntimeStop();

		StopAnimatorsRuntime();
		OnPhysicsSimulationStop();
	}

	void Scene::OnPhysicsSimulationStart()
	{
		VX_PROFILE_FUNCTION();

		Physics::OnSimulationStart(this);
		Physics2D::OnSimulationStart(this);
	}

	void Scene::OnPhysicsSimulationUpdate(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		Physics::OnSimulationUpdate(delta);
		Physics2D::OnSimulationUpdate(delta, this);
	}

	void Scene::OnPhysicsSimulationStop()
	{
		VX_PROFILE_FUNCTION();

		Physics::OnSimulationStop(this);
		Physics2D::OnSimulationStop();
	}

	void Scene::OnUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		const bool updateCurrentFrame = !m_IsPaused || m_StepFrames > 0;

		if (updateCurrentFrame)
		{
			// C++ Entity OnUpdate
			GetAllEntitiesWith<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				nsc.Instance->OnUpdate(delta);
			});

			// C# Entity OnUpdate
			auto view = GetAllEntitiesWith<ScriptComponent>();
			for (const auto e : view)
			{
				Entity entity{ e, this };

				if (!entity.IsActive())
					continue;

				ScriptEngine::OnUpdateEntity(entity, delta);
			}

			// Update Physics Bodies
			OnPhysicsSimulationUpdate(delta);

			// Update Animators
			OnAnimatorUpdateRuntime(delta);

			if (m_StepFrames)
			{
				m_StepFrames--;
			}
		}

		// Locate the scene's primary camera
		SceneCamera* primarySceneCamera = nullptr;
		TransformComponent primarySceneCameraTransform;

		Entity primaryCameraEntity = GetPrimaryCameraEntity();

		if (primaryCameraEntity)
		{
			auto& cameraComponent = primaryCameraEntity.GetComponent<CameraComponent>();
			primarySceneCamera = &cameraComponent.Camera;
			primarySceneCameraTransform = GetWorldSpaceTransform(primaryCameraEntity);

			// Set clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);

			if (primarySceneCamera)
			{
				SceneRenderPacket renderPacket{};
				renderPacket.MainCamera = primarySceneCamera;
				renderPacket.MainCameraViewMatrix = Math::Inverse(primarySceneCameraTransform.GetTransform());
				renderPacket.MainCameraProjectionMatrix = primarySceneCamera->GetProjectionMatrix();
				renderPacket.MainCameraWorldSpaceTranslation = primarySceneCameraTransform.Translation;
				renderPacket.TargetFramebuffer = m_TargetFramebuffer;
				renderPacket.Scene = this;
				renderPacket.EditorScene = false;
				s_SceneRenderer.RenderScene(renderPacket);
			}
		}

		// Update Components
		OnMeshUpdateRuntime();

		SystemManager::GetAssetSystem<ParticleSystem>()->OnUpdateRuntime(delta);

		if (updateCurrentFrame)
		{
			UpdateQueueFreeTimers(delta);
		}

		ExecutePostUpdateQueue();
	}

	void Scene::OnUpdateSimulation(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		if (!m_IsPaused || m_StepFrames > 0)
		{
			OnPhysicsSimulationUpdate(delta);

			// Update Animators
			OnAnimatorUpdateRuntime(delta);

			if (m_StepFrames)
				m_StepFrames--;
		}

		// Render
		SceneRenderPacket renderPacket{};
		renderPacket.MainCamera = camera;
		renderPacket.MainCameraViewMatrix = camera->GetViewMatrix();
		renderPacket.MainCameraProjectionMatrix = camera->GetProjectionMatrix();
		renderPacket.TargetFramebuffer = m_TargetFramebuffer;
		renderPacket.Scene = this;
		renderPacket.EditorScene = true;
		s_SceneRenderer.RenderScene(renderPacket);

		// Update Components
		OnMeshUpdateRuntime();
		SystemManager::GetAssetSystem<ParticleSystem>()->OnUpdateRuntime(delta);
	}

	void Scene::OnUpdateEditor(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		// Update Animators
		OnAnimatorUpdateRuntime(delta);

		// Render
		{
			SceneRenderPacket renderPacket{};
			renderPacket.MainCamera = camera;
			renderPacket.MainCameraViewMatrix = camera->GetViewMatrix();
			renderPacket.MainCameraProjectionMatrix = camera->GetProjectionMatrix();
			renderPacket.TargetFramebuffer = m_TargetFramebuffer;
			renderPacket.Scene = this;
			renderPacket.EditorScene = true;
			s_SceneRenderer.RenderScene(renderPacket);
		}

		Entity primaryCameraEntity = GetPrimaryCameraEntity();
		if (primaryCameraEntity)
		{
			const auto& cameraComponent = primaryCameraEntity.GetComponent<CameraComponent>();

			// Set Clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);
		}

		// Update Components
		OnMeshUpdateRuntime();
		SystemManager::GetAssetSystem<ParticleSystem>()->OnUpdateRuntime(delta);
	}

	void Scene::SubmitToPostUpdateQueue(const std::function<void()>& func)
	{
		VX_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_PostUpdateQueueMutex);

		m_PostUpdateQueue.push_back(func);
	}

	void Scene::ExecutePostUpdateQueue()
	{
		VX_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_PostUpdateQueueMutex);

		for (const auto& func : m_PostUpdateQueue)
		{
			func();
		}

		m_PostUpdateQueue.clear();
	}

	void Scene::OnUpdateEntityGui()
	{
		VX_PROFILE_FUNCTION();

		if (!m_IsRunning)
			return;

		auto view = GetAllEntitiesWith<ScriptComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			ScriptEngine::OnGuiEntity(entity);
		}
	}

	void Scene::SetPaused(bool paused)
	{
		VX_CORE_ASSERT(m_IsRunning, "Scene must be running!");

		if ((m_IsPaused && paused) || (!m_IsPaused && !paused))
			return;

		m_IsPaused = paused;

		switch (m_IsPaused)
		{
			case true:  SystemManager::OnRuntimeScenePaused();  break;
			case false: SystemManager::OnRuntimeSceneResumed(); break;
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		VX_PROFILE_FUNCTION();

		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize non-FixedAspectRatio cameras
		auto view = GetAllEntitiesWith<CameraComponent>();

		for (const auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);

			if (cameraComponent.FixedAspectRatio)
				continue;

			cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	void Scene::ParentEntity(Entity entity, Entity parent)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(entity, "Entity was invalid!");
		VX_CORE_ASSERT(parent, "Parent was invalid!");

		if (parent.IsDescendantOf(entity))
		{
			UnparentEntity(parent);

			Entity newParent = TryGetEntityWithUUID(entity.GetParentUUID());
			if (newParent)
			{
				UnparentEntity(entity);
				ParentEntity(parent, newParent);
			}
		}
		else
		{
			Entity previousParent = TryGetEntityWithUUID(entity.GetParentUUID());
			if (previousParent)
			{
				UnparentEntity(entity);
			}
		}

		entity.SetParentUUID(parent.GetUUID());
		parent.Children().push_back(entity.GetUUID());

		ConvertToLocalSpace(entity);
	}

	void Scene::UnparentEntity(Entity entity, bool convertToWorldSpace)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(entity, "Entity was invalid!");

		Entity parent = entity.GetParent();
		VX_CORE_ASSERT(parent, "Parent was invalid!");
		if (!parent)
			return;

		auto& parentChildren = parent.Children();
		parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), entity.GetUUID()), parentChildren.end());

		if (convertToWorldSpace)
			ConvertToWorldSpace(entity);

		entity.SetParentUUID(0);
	}

	void Scene::ActiveateChildren(Entity entity)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(entity, "Entity was invalid!");

		const std::vector<UUID>& children = entity.Children();

		for (const auto& child : children)
		{
			Entity childEntity = TryGetEntityWithUUID(child);

			if (!childEntity)
				continue;

			childEntity.SetActive(true);

			if (!childEntity.Children().empty())
				ActiveateChildren(childEntity);
		}
	}

	void Scene::DeactiveateChildren(Entity entity)
	{
		VX_PROFILE_FUNCTION();
		VX_CORE_ASSERT(entity, "Entity was invalid!");

		const std::vector<UUID>& children = entity.Children();

		for (const auto& child : children)
		{
			Entity childEntity = TryGetEntityWithUUID(child);

			if (!childEntity)
				continue;

			childEntity.SetActive(false);

			if (!childEntity.Children().empty())
				DeactiveateChildren(childEntity);
		}
	}

	Entity Scene::GetRootEntityInHierarchy(Entity child) const
	{
		VX_PROFILE_FUNCTION();

		if (!child.HasParent())
			return child;

		Entity parent = child.GetParent();
		return GetRootEntityInHierarchy(parent);
	}

	Entity Scene::TryGetEntityWithUUID(UUID uuid)
	{
		VX_PROFILE_FUNCTION();

		if (auto it = m_EntityMap.find(uuid); it != m_EntityMap.end())
			return Entity{ it->second, this };

		return Entity{};
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<CameraComponent>();

		for (const auto entity : view)
		{
			const auto& cc = view.get<CameraComponent>(entity);

			if (cc.Primary)
				return { entity, this };
		}

		return Entity{};
	}

	Entity Scene::GetSkyLightEntity()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<LightSourceComponent>();

		for (const auto entity : view)
		{
			auto& lsc = view.get<LightSourceComponent>(entity);

			if (lsc.Type != LightType::Directional)
				continue;

			return { entity, this };
		}

		return Entity{};
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		if (!entity)
			return Entity{};

		auto ParentNewEntityFunc = [&entity, scene = this](Entity newEntity)
		{
			if (auto parent = entity.GetParent(); parent)
			{
				newEntity.SetParentUUID(parent.GetUUID());
				parent.AddChild(newEntity.GetUUID());
			}
		};

		// TODO: handle prefabs
		/*if (src.HasComponent<PrefabComponent>())
		{
			auto prefabID = src.GetComponent<PrefabComponent>().PrefabUUID;
		}*/
		
		Entity newEntity;

		if (entity.HasComponent<TagComponent>())
		{
			newEntity = CreateEntity(entity.GetName(), entity.GetMarker());
		}
		else
		{
			newEntity = CreateEntity();
			TagComponent& tagComponent = newEntity.GetComponent<TagComponent>();
			tagComponent.Tag = entity.GetName();
			tagComponent.Marker = entity.GetMarker();
		}

		// Copy components (except IDComponent and TagComponent)
		Utils::CopyComponentIfExists(AllComponents{}, newEntity, entity);

		// We must copy here because the vector is modified below
		auto childIDs = entity.Children();

		for (auto childID : childIDs)
		{
			Entity child = TryGetEntityWithUUID(childID);
			Entity childDuplicate = DuplicateEntity(child);

			// at this point childDuplicate is a child of src, we need to remove it from src
			UnparentEntity(childDuplicate, false);
			ParentEntity(childDuplicate, newEntity);
		}

		ParentNewEntityFunc(newEntity);

		// TODO Duplicate script instance

		return newEntity;
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<TagComponent>();

		for (const auto entity : view)
		{
			const auto& tag = view.get<TagComponent>(entity).Tag;
			if (strcmp(name.data(), tag.c_str()) == 0)
				return Entity{ entity, this };
		}

		return Entity{};
	}

	Entity Scene::FindEntityWithID(entt::entity entity)
	{
		VX_PROFILE_FUNCTION();

		m_Registry.each([&](auto e)
		{
			if (e == entity)
				return Entity{ e, this };
		});

		return Entity{};
	}

	void Scene::ConvertToLocalSpace(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		Entity parent = TryGetEntityWithUUID(entity.GetParentUUID());

		if (!parent)
		{
			return;
		}

		auto& transform = entity.GetTransform();
		Math::mat4 parentTransform = GetWorldSpaceTransformMatrix(parent);
		Math::mat4 localTransform = Math::Inverse(parentTransform) * transform.GetTransform();
		transform.SetTransform(localTransform);
	}

	void Scene::ConvertToWorldSpace(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		Entity parent = TryGetEntityWithUUID(entity.GetParentUUID());

		if (!parent)
		{
			return;
		}

		Math::mat4 transform = GetWorldSpaceTransformMatrix(entity);
		auto& entityTransform = entity.GetTransform();
		entityTransform.SetTransform(transform);
	}

	Math::mat4 Scene::GetWorldSpaceTransformMatrix(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 transform(1.0f);

		UUID parentUUID = entity.GetParentUUID();
		Entity parent = TryGetEntityWithUUID(parentUUID);

		if (parent)
		{
			transform = GetWorldSpaceTransformMatrix(parent);
		}

		return transform * entity.GetTransform().GetTransform();
	}

	TransformComponent Scene::GetWorldSpaceTransform(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 transform = GetWorldSpaceTransformMatrix(entity);
		TransformComponent transformComponent;
		transformComponent.SetTransform(transform);
		return transformComponent;
	}

    SharedReference<Scene::SceneGeometry>& Scene::GetSceneMeshes()
	{
		if (!m_SceneMeshes)
			m_SceneMeshes = SharedReference<SceneGeometry>::Create();

		ClearSceneMeshes();

		auto meshView = GetAllEntitiesWith<MeshRendererComponent>();

		for (const auto meshRenderer : meshView)
		{
			Entity entity{ meshRenderer, this };
			const auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

			// Skip if not active
			if (!entity.IsActive())
				continue;

			if (!meshRendererComponent.Visible)
				continue;

			AssetHandle meshHandle = meshRendererComponent.Mesh;
			if (!AssetManager::IsHandleValid(meshHandle))
				continue;

			SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
			if (!mesh)
				continue;

			m_SceneMeshes->MeshEntities.push_back(entity);

			m_SceneMeshes->Meshes.push_back(mesh);

			Math::mat4 worldSpaceTransform = GetWorldSpaceTransformMatrix(entity);
			m_SceneMeshes->WorldSpaceMeshTransforms.push_back(worldSpaceTransform);
		}

		auto staticMeshView = GetAllEntitiesWith<StaticMeshRendererComponent>();

		for (const auto staticMeshRenderer : staticMeshView)
		{
			Entity entity{ staticMeshRenderer, this };
			const auto& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

			// Skip if not active
			if (!entity.IsActive())
				continue;

			if (!staticMeshRendererComponent.Visible)
				continue;

			AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
			if (!AssetManager::IsHandleValid(staticMeshHandle))
				continue;

			SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
			if (!staticMesh)
				continue;

			m_SceneMeshes->StaticMeshes.push_back(staticMesh);

			Math::mat4 worldSpaceTransform = GetWorldSpaceTransformMatrix(entity);
			m_SceneMeshes->WorldSpaceStaticMeshTransforms.push_back(worldSpaceTransform);
		}

		return m_SceneMeshes;
	}

	bool Scene::AreEntitiesRelated(Entity first, Entity second)
	{
		VX_PROFILE_FUNCTION();

		return first.IsAncesterOf(second) || second.IsAncesterOf(first);
	}

	void Scene::SortEntities()
	{
		VX_PROFILE_FUNCTION();

		m_Registry.sort<IDComponent>([&](const auto lhs, const auto rhs)
		{
			auto lhsEntity = m_EntityMap.find(lhs.ID);
			auto rhsEntity = m_EntityMap.find(rhs.ID);
			return static_cast<uint32_t>(lhsEntity->second) < static_cast<uint32_t>(rhsEntity->second);
		});
	}

	void Scene::SetSceneCameraViewportSize()
	{
		if (Entity primaryCamera = GetPrimaryCameraEntity())
		{
			CameraComponent& cameraComponent = primaryCamera.GetComponent<CameraComponent>();
			SceneCamera& sceneCamera = cameraComponent.Camera;
			sceneCamera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}
	}

	void Scene::CreateScriptInstancesRuntime()
	{
		VX_PROFILE_FUNCTION();

		// C# Entity OnCreate
		{
			ScriptEngine::OnRuntimeStart(this);

			auto view = GetAllEntitiesWith<ScriptComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::CreateEntityScriptInstanceRuntime(entity);
			}

			for (const auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::OnAwakeEntity(entity);
			}

			for (const auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::OnCreateEntity(entity);
			}
		}

		// C++ Entity OnCreate
		{
			GetAllEntitiesWith<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			});
		}
	}

	void Scene::DestroyScriptInstancesRuntime()
	{
		VX_PROFILE_FUNCTION();

		GetAllEntitiesWith<ScriptComponent>().each([=](auto entityID, auto& scriptComponent)
		{
			Entity entity{ entityID, this };
			ScriptEngine::OnDestroyEntity(entity);
		});
	}

	void Scene::StopAnimatorsRuntime()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<TransformComponent, AnimatorComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;

			if (!animator->IsPlaying())
				continue;

			animator->Stop();
		}
	}

	void Scene::OnMeshUpdateRuntime()
	{
		VX_PROFILE_FUNCTION();

		{
			auto view = GetAllEntitiesWith<MeshRendererComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, this };
				MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

				AssetHandle meshHandle = meshRendererComponent.Mesh;
				if (!AssetManager::IsHandleValid(meshHandle))
					continue;

				SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);
				if (!mesh)
					continue;

				mesh->OnUpdate((int)(entt::entity)e);
			}
		}

		{
			auto view = GetAllEntitiesWith<StaticMeshRendererComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, this };
				StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

				AssetHandle staticMeshHandle = staticMeshRendererComponent.StaticMesh;
				if (!AssetManager::IsHandleValid(staticMeshHandle))
					continue;

				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshHandle);
				if (!staticMesh)
					continue;

				auto& materialTable = staticMeshRendererComponent.Materials;

				staticMesh->OnUpdate(materialTable, (int)(entt::entity)e);
			}
		}
	}

	void Scene::OnAnimatorUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<AnimatorComponent, AnimationComponent, MeshRendererComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;

			if (!animator || !animator->IsPlaying())
				continue;

			animator->UpdateAnimation(delta);
		}
	}

	void Scene::ClearSceneMeshes()
	{
		m_SceneMeshes->MeshEntities.clear();
		m_SceneMeshes->Meshes.clear();
		m_SceneMeshes->WorldSpaceMeshTransforms.clear();
		m_SceneMeshes->StaticMeshes.clear();
		m_SceneMeshes->WorldSpaceStaticMeshTransforms.clear();
	}

	void Scene::OnCameraConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

		if (m_ViewportWidth != 0 && m_ViewportHeight != 0)
			cameraComponent.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		if (cameraComponent.Primary)
			RenderCommand::SetClearColor(cameraComponent.ClearColor);
	}

	void Scene::OnStaticMeshConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		StaticMeshRendererComponent& staticMeshComponent = entity.GetComponent<StaticMeshRendererComponent>();

		if (staticMeshComponent.Type != MeshType::Custom)
		{
			DefaultMeshes::StaticMeshes staticMeshType = (DefaultMeshes::StaticMeshes)staticMeshComponent.Type;

			staticMeshComponent.StaticMesh = Project::GetEditorAssetManager()->GetDefaultStaticMesh(staticMeshType);

			SharedReference<MaterialTable> materialTable = staticMeshComponent.Materials;

			if (AssetManager::IsHandleValid(staticMeshComponent.StaticMesh) && materialTable->Empty())
			{
				SharedReference<StaticMesh> staticMesh = AssetManager::GetAsset<StaticMesh>(staticMeshComponent.StaticMesh);
				staticMesh->LoadMaterialTable(materialTable);
			}
		}
	}

	void Scene::OnParticleEmitterConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		const ParticleEmitterComponent& pmc = entity.GetComponent<ParticleEmitterComponent>();
		if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			SystemManager::GetAssetSystem<ParticleSystem>()->CreateAsset(entity);
	}

	void Scene::OnParticleEmitterDestruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		SystemManager::GetAssetSystem<ParticleSystem>()->DestroyAsset(entity);
	}

	void Scene::OnTextMeshConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();

		if (!AssetManager::IsHandleValid(textMeshComponent.FontAsset))
		{
			textMeshComponent.FontAsset = Font::GetDefaultFont()->Handle;
		}
	}

	void Scene::OnAnimatorConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		AnimatorComponent& animatorComponent = entity.GetComponent<AnimatorComponent>();

		if (entity.HasComponent<AnimationComponent>())
		{
			const AnimationComponent& animationComponent = entity.GetComponent<AnimationComponent>();
			animatorComponent.Animator = Animator::Create(animationComponent.Animation);
		}
	}

	void Scene::OnAnimationConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		const AnimationComponent& animationComponent = entity.GetComponent<AnimationComponent>();

		if (entity.HasComponent<MeshRendererComponent>())
		{
			auto& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();
			AssetHandle meshHandle = meshRendererComponent.Mesh;

			if (AssetManager::IsHandleValid(meshHandle))
			{
				SharedReference<Mesh> mesh = AssetManager::GetAsset<Mesh>(meshHandle);

				if (mesh)
				{
					// TODO fix this
					//component.Animation = Animation::Create(filepath, mesh->Handle);
				}
			}
		}
	}

	void Scene::OnAudioSourceConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		const AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
		if (!AssetManager::IsHandleValid(asc.AudioHandle))
		{
			// TODO
		}
	}

	void Scene::OnAudioSourceDestruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		// TODO
	}

	void Scene::OnAudioListenerConstruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		const AudioListenerComponent& alc = entity.GetComponent<AudioListenerComponent>();
		if (!AssetManager::IsHandleValid(alc.ListenerHandle))
		{
			// TODO
		}
	}

	void Scene::OnAudioListenerDestruct(entt::registry& registry, entt::entity e)
	{
		Entity entity = { e, this };
		// TODO
	}

	void Scene::SubmitSceneToBuild(const std::string& sceneFilePath)
	{
		VX_PROFILE_FUNCTION();

		Project::SubmitSceneToBuild(sceneFilePath);
	}

	void Scene::RemoveIndexFromBuild(uint32_t buildIndex)
	{
		BuildIndexMap& buildIndices = Project::GetScenesInBuild();

		if (buildIndices.contains(buildIndex))
		{
			buildIndices.erase(buildIndex);
		}
	}

	const BuildIndexMap& Scene::GetScenesInBuild()
	{
		return Project::GetScenesInBuild();
	}

    uint32_t Scene::GetActiveSceneBuildIndex()
    {
		return s_ActiveBuildIndex;
    }

	void Scene::SetActiveSceneBuildIndex(uint32_t buildIndex)
	{
		s_ActiveBuildIndex = buildIndex;
	}

	SharedReference<Scene> Scene::Copy(SharedReference<Scene>& source)
	{
		VX_PROFILE_FUNCTION();

		SharedReference<Scene> destination = Scene::Create(source->m_TargetFramebuffer);

		destination->m_TargetFramebuffer = source->m_TargetFramebuffer;
		destination->m_ViewportWidth = source->m_ViewportWidth;
		destination->m_ViewportHeight = source->m_ViewportHeight;

		auto& srcSceneRegistry = source->m_Registry;
		auto& dstSceneRegistry = destination->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entites in new scene
		auto view = srcSceneRegistry.view<IDComponent>();
		for (const auto e : view)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& tagComponent = srcSceneRegistry.get<TagComponent>(e);
			const auto& name = tagComponent.Tag;
			const auto& marker = tagComponent.Marker;
			Entity copiedEntity = destination->CreateEntityWithUUID(uuid, name, marker);
			copiedEntity.SetActive(tagComponent.IsActive);
			enttMap[uuid] = (entt::entity)copiedEntity;
		}

		// Copy all components (except IDComponent and TagComponent)
		Utils::CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		destination->SortEntities();

		return destination;
	}

	void Scene::Create2DSampleScene(SharedReference<Scene>& context)
	{
		// Starting Entities
		Entity startingCamera = context->CreateEntity("Camera");
		startingCamera.AddComponent<AudioListenerComponent>();
		SceneCamera& camera = startingCamera.AddComponent<CameraComponent>().Camera;
		camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
		TransformComponent& cameraTransform = startingCamera.GetTransform();
		cameraTransform.Translation = { 0.0f, 0.0f, 0.0f };
		cameraTransform.SetRotationEuler({ 0.0f, 0.0f, 0.0f });
	}

	void Scene::Create3DSampleScene(SharedReference<Scene>& context)
	{
		// Starting Entities
		Entity startingCube = context->CreateEntity("Cube");
		startingCube.AddComponent<StaticMeshRendererComponent>();
		startingCube.AddComponent<RigidBodyComponent>();
		startingCube.AddComponent<BoxColliderComponent>();

		Entity startingSkyLight = context->CreateEntity("Sky Light");
		LightSourceComponent& lightSource = startingSkyLight.AddComponent<LightSourceComponent>();
		lightSource.Type = LightType::Directional;
		startingSkyLight.GetTransform().SetRotationEuler({ 0.0f, Math::Deg2Rad(-57.0f), 0.0f });
		startingSkyLight.GetTransform().Translation = { -1.0f, 5.0f, 1.0f };

		Entity startingCamera = context->CreateEntity("Camera");
		// we can do this once audio is fixed
		//startingCamera.AddComponent<AudioListenerComponent>();
		SceneCamera& camera = startingCamera.AddComponent<CameraComponent>().Camera;
		camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		TransformComponent& cameraTransform = startingCamera.GetTransform();
		cameraTransform.Translation = { -4.0f, 3.0f, 4.0f };
		cameraTransform.SetRotationEuler({ Math::Deg2Rad(-25.0f), Math::Deg2Rad(-45.0f), 0.0f });
	}

	SharedReference<Scene> Scene::Create(SharedReference<Framebuffer>& targetFramebuffer)
	{
		return SharedReference<Scene>::Create(targetFramebuffer);
	}

	SharedReference<Scene> Scene::Create()
	{
		return SharedReference<Scene>::Create();
	}

}

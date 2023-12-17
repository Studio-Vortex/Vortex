#include "vxpch.h"
#include "Scene.h"

#include "Vortex/Core/Application.h"
#include "Vortex/Core/String.h"

#include "Vortex/Math/Math.h"

#include "Vortex/Asset/AssetManager.h"

#include "Vortex/Project/Project.h"

#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scene/SceneRenderer.h"
#include "Vortex/Scene/ScriptableEntity.h"

#include "Vortex/Audio/Audio.h"
#include "Vortex/Audio/AudioSource.h"

#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"

#include "Vortex/UI/UISystem.h"

#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/ParticleSystem/ParticleSystem.h"
#include "Vortex/Renderer/ParticleSystem/ParticleEmitter.h"

#include "Vortex/System/SystemManager.h"

#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Scripting/RuntimeMethodArgument.h"

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
			}(), ...);
		}

		template<typename... TComponent>
		static void CopyComponentIfExists(ComponentGroup<TComponent...>, Entity dst, Entity src)
		{
			CopyComponentIfExists<TComponent...>(dst, src);
		}

	}

	static SceneRenderer s_SceneRenderer;
	static QueueFreeData s_NullQueueFreeData;
	static Timer s_NullTimer = Timer("", 0.0f, nullptr);

	Scene::Scene(SharedReference<Framebuffer>& targetFramebuffer)
		: m_TargetFramebuffer(targetFramebuffer)
	{
		SystemManager::OnContextSceneCreated(this);

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
		SystemManager::OnContextSceneDestroyed(this);

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
		// Temporary fix
		// this immediately destroys the entity which is not ideal but works for now
		DestroyEntityInternal(entity, excludeChildren);

		// TODO figure out why this doesn't work
		// When the lambda is passed over to the queue, all of the entity's information is lost,
		// i.e. the scene pointer etc...
		// This most likely has to do with the way the arguments are passed on the stack

		// It would be safest to wait until the end of the current frame to delete the entity
		// otherwise something may reference it later on during ::OnUpdateRuntime()
		
		//auto OnDestroyedFn = [&]() { DestroyEntityInternal(entity, excludeChildren); };
		//SubmitToPostUpdateQueue(OnDestroyedFn);
	}

	void Scene::SubmitToDestroyEntity(const QueueFreeData& queueFreeData)
	{
		DestroyEntityInternal(queueFreeData);
	}

	void Scene::ClearEntities()
	{
		m_Registry.clear();
	}

	const QueueFreeData& Scene::GetQueueFreeStatus(UUID entityUUID) const
	{
		VX_CORE_ASSERT(m_QueueFreeMap.contains(entityUUID), "entity was not found in queue free map!");

		if (auto it = m_QueueFreeMap.find(entityUUID); it != m_QueueFreeMap.end())
		{
			return m_QueueFreeMap.at(entityUUID);
		}

		return s_NullQueueFreeData;
	}

	void Scene::DestroyEntityInternal(Entity entity, bool excludeChildren)
	{
		VX_PROFILE_FUNCTION();

#ifdef VX_DEBUG
		const uint32_t garbage = 0xcccccccc;
		uint32_t addr = (uint32_t)entity.GetContextScene();
		if (addr == garbage)
		{
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyEntityInternal with invalid Scene!");
			return;
		}
		addr = (uint32_t)entity.operator entt::entity();
		if (addr == garbage)
		{
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyEntityInternal with invalid Entity!");
			return;
		}
#endif

		if (!entity || !m_EntityMap.contains(entity))
		{
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyEntityInternal with invalid Entity!");
			VX_CORE_ASSERT(false, "Trying to free invalid Entity!");
			return;
		}

		if (Entity selected = SelectionManager::GetSelectedEntity(); selected == entity)
		{
			SelectionManager::DeselectEntity();
		}

		if (m_IsRunning)
		{
			// Invoke Entity.OnDestroy
			if (entity.HasComponent<ScriptComponent>() && ScriptEngine::GetContextScene() != nullptr)
			{
				const ScriptComponent& scriptComponent = entity.GetComponent<ScriptComponent>();

				if (ScriptEngine::EntityClassExists(scriptComponent.ClassName))
				{
					ScriptEngine::Invoke(ManagedMethod::OnDestroy, entity);
				}
			}

			if (entity.HasComponent<NativeScriptComponent>())
			{
				NativeScriptComponent& nsc = entity.GetComponent<NativeScriptComponent>();
				nsc.Instance->OnDestroy();
				nsc.DestroyInstanceScript(&nsc);
			}

			// Destroy physics body
			Physics::DestroyPhysicsActor(entity);
			Physics2D::DestroyPhysicsBody(entity);
		}

		if (Entity parent = entity.GetParent())
		{
			parent.RemoveChild(entity.GetUUID());
		}

		if (!excludeChildren)
		{
			for (size_t i = 0; i < entity.Children().size(); i++)
			{
				UUID childID = entity.Children()[i];
				Entity child = TryGetEntityWithUUID(childID);
				DestroyEntityInternal(child, excludeChildren);
			}
		}

		auto it = m_EntityMap.find(entity.GetUUID());
		VX_CORE_ASSERT(it != m_EntityMap.end(), "Enitiy was not found in Entity Map!");

		if (it == m_EntityMap.end())
		{
			VX_CONSOLE_LOG_ERROR("Entity was not found in Entity Map!");
			return;
		}

		// Remove the entity from our internal map
		m_EntityMap.erase(it->first);
		m_Registry.destroy(entity);

		SortEntities();
	}

	void Scene::DestroyEntityInternal(const QueueFreeData& queueFreeData)
	{
		VX_PROFILE_FUNCTION();

		if (!m_EntityMap.contains(queueFreeData.EntityUUID))
		{
			return;
		}

		if (m_QueueFreeMap.contains(queueFreeData.EntityUUID))
		{
			return;
		}

		const bool invalidTimer = queueFreeData.WaitTime <= 0.0f;

		if (invalidTimer)
		{
			Entity entity = m_EntityMap[queueFreeData.EntityUUID];
			SubmitToDestroyEntity(entity, queueFreeData.ExcludeChildren);
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyEntityInternal with a wait time of 0, Use the regular method instead!");
			return;
		}

		m_QueueFreeMap[queueFreeData.EntityUUID] = queueFreeData;
	}

	void Scene::OnUpdateQueueFreeTimers(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		// Update timers for entites to be destroyed
		for (auto& [uuid, queueFreeData] : m_QueueFreeMap)
		{
			queueFreeData.WaitTime -= delta;

			// If the timer isn't finished just move on
			const bool timerDone = queueFreeData.WaitTime <= 0.0f;
			if (!timerDone)
				continue;
			
			auto it = std::find(m_EntitiesToBeRemovedFromQueue.begin(), m_EntitiesToBeRemovedFromQueue.end(), uuid);
			if (it != m_EntitiesToBeRemovedFromQueue.end())
				continue;

			// Timer is done so lets add it to queue
			m_EntitiesToBeRemovedFromQueue.push_back(uuid);
		}

		// Destroy entities that are done waiting
		for (const auto& uuid : m_EntitiesToBeRemovedFromQueue)
		{
			VX_CORE_ASSERT(m_EntityMap.contains(uuid), "Invalid Entity UUID!");
			Entity entity = TryGetEntityWithUUID(uuid);
			const QueueFreeData& data = GetQueueFreeStatus(uuid);
			SubmitToDestroyEntity(entity, data.ExcludeChildren);
			m_QueueFreeMap.erase(uuid);
		}

		m_EntitiesToBeRemovedFromQueue.clear();
	}

	void Scene::OnUpdateEntityTimers(TimeStep delta)
	{
		for (auto& [entityUUID, entityTimers] : m_Timers)
		{
			// update all the timers for each entity
			for (Timer& timer : entityTimers)
			{
				if (timer.IsFinished())
				{
					m_FinishedTimers.push_back(timer);
					continue;
				}

				timer.OnUpdate(delta);
			}

			// remove finished timers
			for (Timer& timer : m_FinishedTimers)
			{
				const std::string& timerName = timer.GetName();
				const size_t timerCount = entityTimers.size();

				for (size_t pos = 0; pos < timerCount; pos++)
				{
					const std::string& potential = entityTimers[pos].GetName();

					if (!String::FastCompare(timerName, potential))
						continue;

					entityTimers.erase(entityTimers.begin() + pos);
					break;
				}
			}

			m_FinishedTimers.clear();
		}
	}

	void Scene::OnRuntimeStart(bool muteAudio)
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = true;

		SetSceneCameraViewportSize();

		OnPhysicsSimulationStart();

		// TODO: when we have AudioSources that play on
		// start we need to stop them here if muteAudio is true
		SystemManager::OnRuntimeStart(this);

		// C# Entity Lifecycle
		{
			ScriptEngine::OnRuntimeStart(this);

			auto view = GetAllEntitiesWith<ScriptComponent>();

			// Create all script instances
			for (const auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::RT_CreateEntityScriptInstance(entity);
			}

			// Invoke Entity.OnAwake
			for (const auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::Invoke(ManagedMethod::OnAwake, entity);
			}

			// Invoke Entity.OnCreate
			for (const auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::Invoke(ManagedMethod::OnCreate, entity);
			}
		}

		// C++ Entity Lifecycle
		{
			GetAllEntitiesWith<NativeScriptComponent>().each([=](auto entityID, auto& nsc)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entityID, this };
				nsc.Instance->OnCreate();
			});
		}
	}

	void Scene::OnRuntimeStop()
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = false;

		// Invoke Entity.OnDestroy
		GetAllEntitiesWith<ScriptComponent>().each([=](auto entityID, auto& scriptComponent)
		{
			Entity entity{ entityID, this };
			ScriptEngine::Invoke(ManagedMethod::OnDestroy, entity);
		});
		ScriptEngine::OnRuntimeStop();

		// C++ Entity Lifecycle
		GetAllEntitiesWith<NativeScriptComponent>().each([=](auto entityID, auto& nsc)
		{
			nsc.Instance->OnDestroy();
			nsc.DestroyInstanceScript(&nsc);
		});

		SystemManager::OnRuntimeStop(this);

		StopAnimatorsRuntime();
		OnPhysicsSimulationStop();

		m_Timers.clear();
	}

	void Scene::OnPhysicsSimulationStart()
	{
		VX_PROFILE_FUNCTION();

		m_IsSimulating = true;

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

		m_IsSimulating = false;

		Physics::OnSimulationStop(this);
		Physics2D::OnSimulationStop();
	}

	void Scene::OnUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		ExecutePreUpdateQueue();

		const bool updateCurrentFrame = !m_IsPaused || m_StepFrames > 0;

		if (updateCurrentFrame)
		{
#ifndef VX_DIST
			InstrumentationTimer timer("Scene::OnUpdateRuntime - Script Update");
#endif

			// Update C++ Entity
			GetAllEntitiesWith<NativeScriptComponent>().each([=](auto entityID, auto& nsc)
			{
				Entity entity{ entityID, this };
				
				if (!entity.IsActive())
					return;

				nsc.Instance->OnUpdate(delta);
			});

			// Invoke Entity.OnUpdate
			auto view = GetAllEntitiesWith<ScriptComponent>();
			for (const auto e : view)
			{
				Entity entity{ e, this };

				if (!entity.IsActive())
					continue;

				RuntimeMethodArgument arg0(delta);
				ScriptEngine::Invoke(ManagedMethod::OnUpdate, entity, { arg0 });
			}

#ifndef VX_DIST
			Application& application = Application::Get();
			FrameTime& frameTime = application.GetFrameTime();
			frameTime.ScriptUpdateTime += timer.ElapsedMS();
#endif

#ifndef VX_DIST
			timer = InstrumentationTimer("Scene::OnUpdateRuntime - Physics Update");
#endif
			// Update Physics Bodies
			OnPhysicsSimulationUpdate(delta);

#ifndef VX_DIST
			frameTime.PhysicsUpdateTime += timer.ElapsedMS();
#endif

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

		// Render from the primary camera's point of view
		if (Entity primaryCameraEntity = GetPrimaryCameraEntity())
		{
			CameraComponent& cameraComponent = primaryCameraEntity.GetComponent<CameraComponent>();
			primarySceneCamera = &cameraComponent.Camera;
			primarySceneCameraTransform = GetWorldSpaceTransform(primaryCameraEntity);

			// Set clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);

			if (primarySceneCamera)
			{
				SceneRenderPacket renderPacket{};
				renderPacket.PrimaryCamera = primarySceneCamera;
				renderPacket.PrimaryCameraViewMatrix = Math::Inverse(primarySceneCameraTransform.GetTransform());
				renderPacket.PrimaryCameraProjectionMatrix = primarySceneCamera->GetProjectionMatrix();
				renderPacket.PrimaryCameraWorldSpaceTranslation = primarySceneCameraTransform.Translation;
				renderPacket.TargetFramebuffer = m_TargetFramebuffer;
				renderPacket.Scene = this;
				renderPacket.EditorScene = false;
				s_SceneRenderer.RenderScene(renderPacket);
			}
		}

		// Update Components/Systems
		OnComponentUpdate(delta);
		OnSystemUpdate(delta);

		if (updateCurrentFrame)
		{
			OnUpdateQueueFreeTimers(delta);
			OnUpdateEntityTimers(delta);
		}

		ExecutePostUpdateQueue();
	}

	void Scene::OnUpdateSimulation(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		ExecutePreUpdateQueue();

		if (!m_IsPaused || m_StepFrames > 0)
		{
			OnPhysicsSimulationUpdate(delta);

			// Update Animators
			OnAnimatorUpdateRuntime(delta);

			if (m_StepFrames)
			{
				m_StepFrames--;
			}
		}

		// Render
		SceneRenderPacket renderPacket{};
		renderPacket.PrimaryCamera = camera;
		renderPacket.PrimaryCameraViewMatrix = camera->GetViewMatrix();
		renderPacket.PrimaryCameraProjectionMatrix = camera->GetProjectionMatrix();
		renderPacket.TargetFramebuffer = m_TargetFramebuffer;
		renderPacket.Scene = this;
		renderPacket.EditorScene = true;
		s_SceneRenderer.RenderScene(renderPacket);

		// Update Components/Systems
		OnComponentUpdate(delta);
		OnSystemUpdate(delta);

		ExecutePostUpdateQueue();
	}

	void Scene::OnUpdateEditor(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		ExecutePreUpdateQueue();

		// Update Animators
		OnAnimatorUpdateRuntime(delta);

		// Render
		{
			SceneRenderPacket renderPacket{};
			renderPacket.PrimaryCamera = camera;
			renderPacket.PrimaryCameraViewMatrix = camera->GetViewMatrix();
			renderPacket.PrimaryCameraProjectionMatrix = camera->GetProjectionMatrix();
			renderPacket.TargetFramebuffer = m_TargetFramebuffer;
			renderPacket.Scene = this;
			renderPacket.EditorScene = true;
			s_SceneRenderer.RenderScene(renderPacket);
		}

		if (Entity primaryCameraEntity = GetPrimaryCameraEntity())
		{
			const CameraComponent& cameraComponent = primaryCameraEntity.GetComponent<CameraComponent>();

			// Set Clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);
		}

		// Update Components/Systems
		OnComponentUpdate(delta);
		OnSystemUpdate(delta);

		ExecutePostUpdateQueue();
	}

	void Scene::SubmitToPreUpdateQueue(const std::function<void()>& fn)
	{
		std::scoped_lock<std::mutex> lock(m_PreUpdateQueueMutex);

		m_PreUpdateQueue.emplace_back(fn);
	}

	void Scene::SubmitToPostUpdateQueue(const std::function<void()>& fn)
	{
		std::scoped_lock<std::mutex> lock(m_PostUpdateQueueMutex);

		m_PostUpdateQueue.emplace_back(fn);
	}

	void Scene::ExecutePreUpdateQueue()
	{
		VX_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_PreUpdateQueueMutex);

		for (const auto& fn : m_PreUpdateQueue)
		{
			std::invoke(fn);
		}

		m_PreUpdateQueue.clear();
	}

	void Scene::ExecutePostUpdateQueue()
	{
		VX_PROFILE_FUNCTION();

		std::scoped_lock<std::mutex> lock(m_PostUpdateQueueMutex);

		for (const auto& fn : m_PostUpdateQueue)
		{
			std::invoke(fn);
		}

		m_PostUpdateQueue.clear();
	}

	void Scene::OnComponentUpdate(TimeStep delta)
	{
		OnMeshUpdateRuntime();
	}

	void Scene::OnSystemUpdate(TimeStep delta)
	{
		SystemManager::GetAssetSystem<ParticleSystem>()->OnUpdateRuntime(this, delta);
		SystemManager::GetSystem<UISystem>()->OnUpdateRuntime(this);
	}

	void Scene::OnUpdateEntityGui()
	{
		VX_PROFILE_FUNCTION();

		if (!m_IsRunning)
			return;

		auto view = GetAllEntitiesWith<ScriptComponent>();

		// Invoke Entity.OnGui
		for (const auto e : view)
		{
			Entity entity{ e, this };

			if (!entity.IsActive())
				continue;

			ScriptEngine::Invoke(ManagedMethod::OnGui, entity);
		}
	}

	void Scene::SetPaused(bool paused)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(m_IsRunning, "Scene must be running!");

		const bool consistent = (m_IsPaused && paused) || (!m_IsPaused && !paused);

		if (consistent)
			return;

		m_IsPaused = paused;

		switch ((uint32_t)m_IsPaused)
		{
			case 1: SystemManager::OnRuntimeScenePaused(this);  break;
			case 0: SystemManager::OnRuntimeSceneResumed(this); break;
		}
	}

	size_t Scene::GetScriptEntityCount()
	{
		auto view = GetAllEntitiesWith<ScriptComponent>();

		return view.size();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		VX_PROFILE_FUNCTION();

		if (m_ViewportWidth == width && m_ViewportHeight == height)
			return;

		m_ViewportWidth = width;
		m_ViewportHeight = height;

		// Resize all non-FixedAspectRatio cameras
		auto view = GetAllEntitiesWith<CameraComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

			if (cameraComponent.FixedAspectRatio)
				continue;

			SceneCamera& camera = cameraComponent.Camera;
			camera.SetViewportSize(width, height);
		}
	}

    const Timer& Scene::TryGetTimerByName(Entity entity, const std::string& name)
    {
		return (const Timer&)TryGetMutableTimerByName(entity, name);
    }

	Timer& Scene::TryGetMutableTimerByName(Entity entity, const std::string& name)
	{
		std::vector<Timer> entityTimers = m_Timers[entity];

		for (Timer& timer : entityTimers)
		{
			const std::string& timerName = timer.GetName();
			if (!String::FastCompare(timerName, name))
				continue;

			return timer;
		}

		return s_NullTimer;
	}

	void Scene::AddOrReplaceTimer(Entity entity, Timer&& timer)
	{
		if (Timer& existing = TryGetMutableTimerByName(entity, timer.GetName()); timer != s_NullTimer)
		{
			existing = std::move(timer);
			return;
		}

		m_Timers[entity].push_back(timer);
	}

	void Scene::ParentEntity(Entity entity, Entity parent)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(entity, "Entity was invalid!");
		VX_CORE_ASSERT(parent, "Parent was invalid!");

		if (parent.IsDescendantOf(entity))
		{
			UnparentEntity(parent);

			if (Entity newParent = entity.GetParent())
			{
				UnparentEntity(entity);
				ParentEntity(parent, newParent);
			}
		}
		else if (Entity previousParent = entity.GetParent())
		{
			UnparentEntity(entity);
		}

		entity.SetParentUUID(parent);
		parent.AddChild(entity);

		ConvertToLocalSpace(entity);
	}

	void Scene::UnparentEntity(Entity entity, bool convertToWorldSpace)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(entity, "Entity was invalid!");

		Entity parent = entity.GetParent();
		VX_CORE_ASSERT(parent, "Parent was invalid!");
		if (!parent)
		{
			return;
		}

		std::vector<UUID>& parentChildren = parent.Children();
		parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), entity.GetUUID()), parentChildren.end());

		if (convertToWorldSpace)
		{
			ConvertToWorldSpace(entity);
		}

		entity.SetParentUUID(0);
	}

	void Scene::ActiveateChildren(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(entity, "Entity was invalid!");

		const std::vector<UUID>& children = entity.Children();

		for (const auto& uuid : children)
		{
			Entity child = TryGetEntityWithUUID(uuid);

			if (!child)
				continue;

			child.SetActive(true);

			if (child.Children().size() > 0)
			{
				ActiveateChildren(child);
			}
		}
	}

	void Scene::DeactiveateChildren(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(entity, "Entity was invalid!");

		const std::vector<UUID>& children = entity.Children();

		for (const auto& uuid : children)
		{
			Entity child = TryGetEntityWithUUID(uuid);

			if (!child)
				continue;

			child.SetActive(false);

			if (child.Children().size() > 0)
			{
				DeactiveateChildren(child);
			}
		}
	}

	Entity Scene::GetRootEntityInHierarchy(Entity child) const
	{
		VX_PROFILE_FUNCTION();

		if (!child.HasParent())
		{
			return child;
		}

		Entity parent = child.GetParent();
		return GetRootEntityInHierarchy(parent);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<CameraComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			const CameraComponent& cc = entity.GetComponent<CameraComponent>();

			if (!entity.IsActive())
				continue;

			if (!cc.Primary)
				continue;
			
			return entity;
		}

		return Entity{};
	}

	Entity Scene::GetEnvironmentEntity()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<SkyboxComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };

			if (!entity.IsActive())
				continue;

			return entity;
		}

		return Entity{};
	}

	Entity Scene::GetSkyLightEntity()
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<LightSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			const LightSourceComponent& lsc = entity.GetComponent<LightSourceComponent>();

			if (!entity.IsActive())
				continue;

			if (lsc.Type != LightType::Directional)
				continue;

			return entity;
		}

		return Entity{};
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		if (!entity)
		{
			return Entity{};
		}

		if (entity.HasComponent<PrefabComponent>())
		{
			// TODO: handle prefabs
			auto prefabID = entity.GetComponent<PrefabComponent>().PrefabUUID;
		}
		
		VX_CORE_ASSERT(entity.HasComponent<TagComponent>(), "all entities must have a tag component!");

		Entity duplicate = CreateEntity(entity.GetName(), entity.GetMarker());

		// Copy components (except IDComponent and TagComponent)
		Utils::CopyComponentIfExists(AllComponents{}, duplicate, entity);

		// Copy children entities
		// We must create a copy here because the vector is modified below
		std::vector<UUID> children = entity.Children();

		for (auto childID : children)
		{
			Entity child = TryGetEntityWithUUID(childID);
			if (!child)
				continue;

			Entity childDuplicate = DuplicateEntity(child);

			// at this point childDuplicate is a child of entity, we need to remove it from entity
			UnparentEntity(childDuplicate, false);
			ParentEntity(childDuplicate, duplicate);
		}

		// if the entity has a parent we can make the duplicate a child also
		if (Entity parent = entity.GetParent())
		{
			ParentEntity(duplicate, parent);
		}

		if (m_IsRunning && entity.HasComponent<ScriptComponent>())
		{
			// Create a new Script Instance for the duplicate
			ScriptEngine::RuntimeInstantiateEntity(duplicate);
		}

		return duplicate;
	}

	Entity Scene::TryGetEntityWithUUID(UUID uuid)
	{
		VX_PROFILE_FUNCTION();

		if (auto it = m_EntityMap.find(uuid); it != m_EntityMap.end())
		{
			return it->second;
		}

		return Entity{};
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		VX_PROFILE_FUNCTION();

		auto view = GetAllEntitiesWith<TagComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			const std::string& tag = entity.GetName();
			if (!String::FastCompare(name, tag))
				continue;

			return Entity{ entity, this };
		}

		return Entity{};
	}

	Entity Scene::FindEntityByID(entt::entity entity)
	{
		VX_PROFILE_FUNCTION();

		m_Registry.each([&](auto e)
		{
			if (e == entity)
			{
				return Entity{ e, this };
			}
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

	// This is clearly a design flaw with the renderer, it should already have all of this data but yet we
	// still need to go and gather it ourselves which is inefficient
    SharedReference<SceneGeometry>& Scene::GetSceneMeshes()
	{
		if (m_SceneMeshes == nullptr)
		{
			m_SceneMeshes = SharedReference<SceneGeometry>::Create();
		}

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

			if (!animator)
				continue;

			if (!animator->IsPlaying())
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
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		CameraComponent& cameraComponent = entity.GetComponent<CameraComponent>();

		if (m_ViewportWidth != 0 && m_ViewportHeight != 0)
		{
			SceneCamera& camera = cameraComponent.Camera;
			camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
		}

		if (cameraComponent.Primary)
		{
			RenderCommand::SetClearColor(cameraComponent.ClearColor);
		}
	}

	void Scene::OnStaticMeshConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		StaticMeshRendererComponent& staticMeshComponent = entity.GetComponent<StaticMeshRendererComponent>();

		if (staticMeshComponent.Type != MeshType::Custom)
		{
			DefaultMesh::StaticMeshType staticMeshType = (DefaultMesh::StaticMeshType)staticMeshComponent.Type;

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
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		const ParticleEmitterComponent& pmc = entity.GetComponent<ParticleEmitterComponent>();
		if (!AssetManager::IsHandleValid(pmc.EmitterHandle))
			SystemManager::GetAssetSystem<ParticleSystem>()->CreateAsset(entity);
	}

	void Scene::OnParticleEmitterDestruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		SystemManager::GetAssetSystem<ParticleSystem>()->DestroyAsset(entity);
	}

	void Scene::OnTextMeshConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		TextMeshComponent& textMeshComponent = entity.GetComponent<TextMeshComponent>();

		if (!AssetManager::IsHandleValid(textMeshComponent.FontAsset))
		{
			textMeshComponent.FontAsset = Font::GetDefaultFont()->Handle;
		}
	}

	void Scene::OnAnimatorConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

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
		VX_PROFILE_FUNCTION();

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
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		AudioSourceComponent& asc = entity.GetComponent<AudioSourceComponent>();
		
		if (!AssetManager::IsHandleValid(asc.AudioHandle))
		{
			asc.AudioHandle = AssetManager::CreateMemoryOnlyAsset<AudioSource>();
		}
	}

	void Scene::OnAudioSourceDestruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		// TODO do we need to do anything here?
	}

	void Scene::OnAudioListenerConstruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		const AudioListenerComponent& alc = entity.GetComponent<AudioListenerComponent>();
		if (!AssetManager::IsHandleValid(alc.ListenerHandle))
		{
			// TODO
		}
	}

	void Scene::OnAudioListenerDestruct(entt::registry& registry, entt::entity e)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { e, this };
		// TODO
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

	void Scene::CreateSampleScene(ProjectType type, SharedReference<Scene>& context)
	{
		switch (type)
		{
			case ProjectType::e2D:
			{
				// Starting Entities
				Entity sprite = context->CreateEntity("Sprite");
				sprite.AddComponent<SpriteRendererComponent>();
				sprite.AddComponent<RigidBodyComponent>();
				sprite.AddComponent<BoxColliderComponent>();

				Entity primaryCamera = context->CreateEntity("Camera");
				// we can do this once audio is fixed
				//startingCamera.AddComponent<AudioListenerComponent>();
				SceneCamera& camera = primaryCamera.AddComponent<CameraComponent>().Camera;
				camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
				TransformComponent& cameraTransform = primaryCamera.GetTransform();
				cameraTransform.Translation = { 0.0f, 0.0f, 0.0f };
				cameraTransform.SetRotationEuler({ 0.0f, 0.0f, 0.0f });
				break;
			}
			case ProjectType::e3D:
			{
				// Starting Entities
				Entity cube = context->CreateEntity("Cube");
				cube.AddComponent<StaticMeshRendererComponent>();
				cube.AddComponent<RigidBodyComponent>();
				cube.AddComponent<BoxColliderComponent>();

				Entity skylight = context->CreateEntity("Sky Light");
				LightSourceComponent& lsc = skylight.AddComponent<LightSourceComponent>();
				lsc.Type = LightType::Directional;
				lsc.ShadowBias = 0.0f;
				skylight.GetTransform().SetRotationEuler({ 0.0f, Math::Deg2Rad(-57.0f), 0.0f });
				skylight.GetTransform().Translation = { -1.0f, 5.0f, 1.0f };

				Entity primaryCamera = context->CreateEntity("Primary Camera");
				// ditto
				//startingCamera.AddComponent<AudioListenerComponent>();
				SceneCamera& camera = primaryCamera.AddComponent<CameraComponent>().Camera;
				camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
				TransformComponent& cameraTransform = primaryCamera.GetTransform();
				cameraTransform.Translation = { -4.0f, 3.0f, 4.0f };
				cameraTransform.SetRotationEuler({ Math::Deg2Rad(-25.0f), Math::Deg2Rad(-45.0f), 0.0f });
				break;
			}
		}
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

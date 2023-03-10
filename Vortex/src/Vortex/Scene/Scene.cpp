#include "vxpch.h"
#include "Scene.h"

#include "Vortex/Core/Math.h"
#include "Vortex/Project/Project.h"
#include "Vortex/Audio/AudioEngine.h"
#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/LightSource2D.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scene/SceneRenderer.h"
#include "Vortex/Scene/ScriptableEntity.h"
#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/2D/Physics2D.h"

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

					// Copy the entity's marker
					auto& srcTagComponent = src.get<TagComponent>(srcEntity);
					dst.emplace_or_replace<TagComponent>(dstEntity, srcTagComponent);

					auto& srcComponent = src.get<TComponent>(srcEntity);
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

					// Copy Resources
					{
						if constexpr (std::is_same<TComponent, StaticMeshRendererComponent>())
						{
							const auto& srcMesh = src.GetComponent<StaticMeshRendererComponent>().StaticMesh;
							auto& dstMesh = dst.GetComponent<StaticMeshRendererComponent>().StaticMesh;
								
							dstMesh = StaticMesh::Create(srcMesh->GetPath(), dst.GetTransform(), MeshImportOptions(), (int)(entt::entity)dst);
								
							const auto& submeshes = srcMesh->GetSubmeshes();
							uint32_t i = 0;

							for (const auto& srcSubmesh : submeshes)
							{
								StaticSubmesh& submesh = dstMesh->GetSubmesh(i++);
								submesh.SetMaterial(srcSubmesh.GetMaterial());
							}
						}

						if constexpr (std::is_same<TComponent, SkyboxComponent>())
						{
							const auto& srcSkybox = src.GetComponent<SkyboxComponent>().Source;
							const auto& dstSkybox = dst.GetComponent<SkyboxComponent>().Source;
							Skybox::Copy(dstSkybox, srcSkybox);
						}

						if constexpr (std::is_same<TComponent, LightSourceComponent>())
						{
							const auto& sourceLightSource = src.GetComponent<LightSourceComponent>().Source;
							const auto& destinationLightSource = dst.GetComponent<LightSourceComponent>().Source;
							LightSource::Copy(destinationLightSource, sourceLightSource);
						}

						if constexpr (std::is_same<TComponent, AudioSourceComponent>())
						{
							const auto& sourceAudioSource = src.GetComponent<AudioSourceComponent>().Source;
							const auto& destinationAudioSource = dst.GetComponent<AudioSourceComponent>().Source;
							AudioSource::Copy(destinationAudioSource, sourceAudioSource);
						}

						if constexpr (std::is_same<TComponent, ParticleEmitterComponent>())
						{
							const auto& sourceEmitter = src.GetComponent<ParticleEmitterComponent>().Emitter;
							const auto& destinationEmitter = dst.GetComponent<ParticleEmitterComponent>().Emitter;
							ParticleEmitter::Copy(destinationEmitter, sourceEmitter);
						}

						// If we copy a script component, we should probably copy all of the script field values as well
						if constexpr (std::is_same<TComponent, ScriptComponent>())
						{
							const auto& sourceScriptFieldMap = ScriptEngine::GetScriptFieldMap(src);
							auto& destinationScriptFieldMap = ScriptEngine::GetScriptFieldMap(dst);

							for (const auto& [name, field] : sourceScriptFieldMap)
							{
								destinationScriptFieldMap[name] = field;
							}
						}
					}
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

	Scene::Scene(SharedRef<Framebuffer> targetFramebuffer)
		: m_TargetFramebuffer(targetFramebuffer) { }

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

	void Scene::DestroyEntity(Entity entity, bool excludeChildren)
	{
		VX_PROFILE_FUNCTION();

		if (entity.GetContextScene() != this || !entity)
			return;

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
				DestroyEntity(child, excludeChildren);
			}
		}

		auto it = m_EntityMap.find(entity.GetUUID());

		VX_CORE_ASSERT(it != m_EntityMap.end(), "Enitiy was not found in Entity Map!");

		// Remove the entity from our internal map
		m_EntityMap.erase(it->second);
		m_Registry.destroy(entity);

		SortEntities();
	}

	void Scene::DestroyEntity(const QueueFreeData& data)
	{
		VX_PROFILE_FUNCTION();

		VX_CORE_ASSERT(!m_QueueFreeMap.contains(data.EntityUUID), "Entity was already submitted to be destroyed!");
		VX_CORE_ASSERT(m_EntityMap.contains(data.EntityUUID), "Entity was not found in Scene Entity Map!");

		const bool invalidTimer = data.WaitTime <= 0.0f;

		if (invalidTimer)
		{
			Entity entity = m_EntityMap[data.EntityUUID];
			DestroyEntity(entity, data.ExcludeChildren);
			VX_CONSOLE_LOG_ERROR("Calling Scene::DestroyEntity with a wait time of 0, Use the regular method instead!");
			return;
		}

		if (m_QueueFreeMap.contains(data.EntityUUID))
			return;

		m_QueueFreeMap[data.EntityUUID] = data;
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
				m_EntitiesToBeRemovedFromQueue.push_back(uuid);
			}
		}

		for (const auto& uuid : m_EntitiesToBeRemovedFromQueue)
		{
			VX_CORE_ASSERT(m_EntityMap.contains(uuid), "Invalid Entity UUID!");
			DestroyEntity(m_EntityMap[uuid], m_QueueFreeMap[uuid].ExcludeChildren);
			m_QueueFreeMap.erase(uuid);
		}

		m_EntitiesToBeRemovedFromQueue.clear();
	}

	void Scene::OnRuntimeStart(bool muteAudio)
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = true;

		OnPhysicsSimulationStart();

		// Audio Source - PlayOnStart
		if (!muteAudio)
		{
			StartAudioSourcesRuntime();
		}

		CreateScriptInstancesRuntime();
	}

	void Scene::OnRuntimeStop()
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = false;

		DestroyScriptInstancesRuntime();

		ScriptEngine::OnRuntimeStop();

		StopAudioSourcesRuntime();

		StopAnimatorsRuntime();

		StopParticleEmittersRuntime();

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

		const bool shouldUpdateCurrentFrame = !m_IsPaused || m_StepFrames > 0;

		if (shouldUpdateCurrentFrame)
		{
			// C++ Entity OnUpdate
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				nsc.Instance->OnUpdate(delta);
			});

			// C# Entity OnUpdate
			const auto view = m_Registry.view<ScriptComponent>();
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
		{
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
					renderPacket.MainCameraWorldSpaceTransform = primarySceneCameraTransform;
					renderPacket.TargetFramebuffer = m_TargetFramebuffer;
					renderPacket.Scene = this;
					renderPacket.EditorScene = false;
					s_SceneRenderer.RenderScene(renderPacket);
				}
			}
		}

		// Update Components
		OnMeshUpdateRuntime();
		OnParticleEmitterUpdateRuntime(delta);

		if (shouldUpdateCurrentFrame)
		{
			SubmitToPostUpdateQueue([=]()
			{
				UpdateQueueFreeTimers(delta);
			});
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
		renderPacket.TargetFramebuffer = m_TargetFramebuffer;
		renderPacket.Scene = this;
		renderPacket.EditorScene = true;
		s_SceneRenderer.RenderScene(renderPacket);

		// Update Components
		OnMeshUpdateRuntime();
		OnParticleEmitterUpdateRuntime(delta);
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
		OnParticleEmitterUpdateRuntime(delta);
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

		auto view = m_Registry.view<ScriptComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			ScriptEngine::OnGuiEntity(entity);
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
		auto view = m_Registry.view<CameraComponent>();

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

		Entity parent = entity.GetParent();
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

	Entity Scene::TryGetRootEntityInHierarchy(Entity child) const
	{
		VX_PROFILE_FUNCTION();

		if (!child.HasParent())
			return child;

		Entity parent = child.GetParent();
		return TryGetRootEntityInHierarchy(parent);
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

		auto view = m_Registry.view<CameraComponent>();

		for (const auto entity : view)
		{
			auto& cc = view.get<CameraComponent>(entity);

			if (cc.Primary)
				return { entity, this };
		}

		return Entity{};
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		if (!entity)
			return {};

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

		auto view = m_Registry.view<TagComponent>();

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

		return {};
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

	bool Scene::AreEntitiesRelated(Entity first, Entity second)
	{
		VX_PROFILE_FUNCTION();

		return first.IsAncesterOf(second) || second.IsAncesterOf(second);
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

	void Scene::StartAudioSourcesRuntime()
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<TransformComponent, AudioSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };

			if (!entity.IsActive())
				continue;

			SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
			const auto& audioProps = audioSource->GetProperties();

			if (!audioProps.PlayOnStart)
				continue;

			audioSource->Play();
		}
	}

	void Scene::StopAudioSourcesRuntime()
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<TransformComponent, AudioSourceComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;

			if (!audioSource->IsPlaying())
				continue;

			audioSource->Stop();
		}
	}

	void Scene::CreateScriptInstancesRuntime()
	{
		VX_PROFILE_FUNCTION();

		// C# Entity OnCreate
		{
			ScriptEngine::OnRuntimeStart(this);

			auto view = m_Registry.view<ScriptComponent>();

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
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
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

		m_Registry.view<ScriptComponent>().each([=](auto entityID, auto& scriptComponent)
		{
			Entity entity{ entityID, this };
			ScriptEngine::OnDestroyEntity(entity);
		});
	}

	void Scene::StopAnimatorsRuntime()
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<TransformComponent, AnimatorComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;

			if (!animator->IsPlaying())
				continue;

			animator->Stop();
		}
	}

	void Scene::StopParticleEmittersRuntime()
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<TransformComponent, ParticleEmitterComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			SharedRef<ParticleEmitter> particleEmitter = entity.GetComponent<ParticleEmitterComponent>().Emitter;

			if (!particleEmitter->IsActive())
				continue;

			particleEmitter->Stop();
		}
	}

	void Scene::OnMeshUpdateRuntime()
	{
		VX_PROFILE_FUNCTION();

		{
			auto view = m_Registry.view<MeshRendererComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, this };
				MeshRendererComponent& meshRendererComponent = entity.GetComponent<MeshRendererComponent>();

				SharedRef<Mesh> mesh = meshRendererComponent.Mesh;
				if (!mesh)
					continue;

				mesh->OnUpdate((int)(entt::entity)e);
			}
		}

		{
			auto view = m_Registry.view<StaticMeshRendererComponent>();

			for (const auto e : view)
			{
				Entity entity{ e, this };
				StaticMeshRendererComponent& staticMeshRendererComponent = entity.GetComponent<StaticMeshRendererComponent>();

				SharedRef<StaticMesh> staticMesh = staticMeshRendererComponent.StaticMesh;
				if (!staticMesh)
					continue;

				staticMesh->OnUpdate((int)(entt::entity)e);
			}
		}
	}

	void Scene::OnAnimatorUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<AnimatorComponent, AnimationComponent, MeshRendererComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;

			if (!animator || !animator->IsPlaying())
				continue;

			animator->UpdateAnimation(delta);
		}
	}

	void Scene::OnParticleEmitterUpdateRuntime(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<ParticleEmitterComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, this };
			SharedRef<ParticleEmitter> particleEmitter = entity.GetComponent<ParticleEmitterComponent>().Emitter;

			// Set the starting particle position to the entity's translation
			Math::vec3 worldSpaceTranslation = GetWorldSpaceTransform(entity).Translation;
			particleEmitter->GetProperties().Position = worldSpaceTranslation;
			particleEmitter->OnUpdate(delta);

			if (!particleEmitter->IsActive())
				continue;
			
			particleEmitter->EmitParticle();
		}
	}

	template <typename TComponent>
	void Scene::OnComponentAdded(Entity entity, TComponent& component)
	{
		static_assert(sizeof(TComponent) != 0);
	}

	template <> void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) { }
	
	template <> void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) { }

	template <> void Scene::OnComponentAdded<HierarchyComponent>(Entity entity, HierarchyComponent& component) { }
	
	template <> void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) { }

	template <> void Scene::OnComponentAdded<PrefabComponent>(Entity entity, PrefabComponent& component) { }

	template <> void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (m_ViewportWidth != 0 && m_ViewportHeight != 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);

		if (component.Primary)
			RenderCommand::SetClearColor(component.ClearColor);
	}

	template <> void Scene::OnComponentAdded<SkyboxComponent>(Entity entity, SkyboxComponent& component)
	{
		component.Source = Skybox::Create();
	}

	template <> void Scene::OnComponentAdded<LightSourceComponent>(Entity entity, LightSourceComponent& component)
	{
		component.Source = LightSource::Create(LightSourceProperties());
	}

	template <> void Scene::OnComponentAdded<LightSource2DComponent>(Entity entity, LightSource2DComponent& component)
	{
		component.Source = LightSource2D::Create(LightSource2DProperties());
	}

	template <> void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component) { }

	template <> void Scene::OnComponentAdded<StaticMeshRendererComponent>(Entity entity, StaticMeshRendererComponent& component)
	{
		StaticMesh::Default defaultModel = StaticMesh::Default::Cube;
		MeshImportOptions importOptions = MeshImportOptions();

		if (component.Type == MeshType::Custom)
		{
			defaultModel = StaticMesh::Default::Cube;
		}
		else if (component.Type == MeshType::Capsule)
		{
			MeshImportOptions importOptions = MeshImportOptions();
			importOptions.MeshTransformation.SetRotationEuler({ 0.0f, 0.0f, 90.0f });
		}
		else
		{
			defaultModel = (StaticMesh::Default)component.Type;
		}

		component.StaticMesh = StaticMesh::Create(defaultModel, entity.GetTransform(), importOptions, (int)(entt::entity)entity);
	}

	template <> void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) { }

	template <> void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) { }

	template <> void Scene::OnComponentAdded<ParticleEmitterComponent>(Entity entity, ParticleEmitterComponent& component)
	{
		component.Emitter = ParticleEmitter::Create(ParticleEmitterProperties());
	}

	template <> void Scene::OnComponentAdded<TextMeshComponent>(Entity entity, TextMeshComponent& component)
	{
		component.FontAsset = Font::GetDefaultFont();
	}
	
	template <> void Scene::OnComponentAdded<AnimatorComponent>(Entity entity, AnimatorComponent& component)
	{
		if (entity.HasComponent<AnimationComponent>())
		{
			component.Animator = Animator::Create(entity.GetComponent<AnimationComponent>().Animation);
		}
	}

	template <> void Scene::OnComponentAdded<AnimationComponent>(Entity entity, AnimationComponent& component)
	{
		if (entity.HasComponent<MeshRendererComponent>() && entity.GetComponent<MeshRendererComponent>().Mesh->HasAnimations())
		{
			SharedRef<Mesh> model = entity.GetComponent<MeshRendererComponent>().Mesh;
			std::string filepath = model->GetPath();
			component.Animation = Animation::Create(filepath, model);
		}
	}

	template <> void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component)
	{
		component.Source = AudioSource::Create();
	}

	template <> void Scene::OnComponentAdded<AudioListenerComponent>(Entity entity, AudioListenerComponent& component)
	{
		component.Listener = AudioListener::Create();
		AudioSource::AddAudioListener();
	}
	
	template <> void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component) { }

	template <> void Scene::OnComponentAdded<CharacterControllerComponent>(Entity entity, CharacterControllerComponent& component) { }

	template <> void Scene::OnComponentAdded<FixedJointComponent>(Entity entity, FixedJointComponent& component) { }

	template <> void Scene::OnComponentAdded<PhysicsMaterialComponent>(Entity entity, PhysicsMaterialComponent& component) { }

	template <> void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<MeshColliderComponent>(Entity entity, MeshColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component) { }

	template <> void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<NavMeshAgentComponent>(Entity entity, NavMeshAgentComponent& component) { }

	template <> void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) { }

	template <> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

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

	SharedRef<Scene> Scene::Copy(SharedRef<Scene>& source)
	{
		VX_PROFILE_FUNCTION();

		SharedRef<Scene> destination = Scene::Create(source->m_TargetFramebuffer);

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
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity copiedEntity = destination->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)copiedEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		Utils::CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		destination->SortEntities();

		return destination;
	}

	void Scene::Create2DSampleScene(SharedRef<Scene>& context)
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

	void Scene::Create3DSampleScene(SharedRef<Scene>& context)
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
		startingCamera.AddComponent<AudioListenerComponent>();
		SceneCamera& camera = startingCamera.AddComponent<CameraComponent>().Camera;
		camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		TransformComponent& cameraTransform = startingCamera.GetTransform();
		cameraTransform.Translation = { -4.0f, 3.0f, 4.0f };
		cameraTransform.SetRotationEuler({ Math::Deg2Rad(-25.0f), Math::Deg2Rad(-45.0f), 0.0f });
	}

	SharedRef<Scene> Scene::Create(SharedRef<Framebuffer> targetFramebuffer)
	{
		return CreateShared<Scene>(targetFramebuffer);
	}

	SharedRef<Scene> Scene::Create()
	{
		return CreateShared<Scene>();
	}

}

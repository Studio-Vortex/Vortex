#include "vxpch.h"
#include "Scene.h"

#include "Vortex/Core/Math.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Audio/AudioEngine.h"
#include "Vortex/Renderer/Renderer.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Renderer/LightSource.h"
#include "Vortex/Renderer/ParticleEmitter.h"
#include "Vortex/Scene/ScriptableEntity.h"
#include "Vortex/Scene/Prefab.h"
#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Asset/AssetRegistry.h"
#include "Vortex/Scene/SceneRenderer.h"
#include "Vortex/Renderer/Font/Font.h"
#include "Vortex/Animation/Animator.h"
#include "Vortex/Animation/Animation.h"
#include "Vortex/Physics/3D/Physics.h"
#include "Vortex/Physics/2D/Physics2D.h"
#include "Vortex/Project/Project.h"

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
			([&]()
				{
					if (src.HasComponent<TComponent>())
					{
						dst.AddOrReplaceComponent<TComponent>(src.GetComponent<TComponent>());

						// Copy Resources
						{
							if constexpr (std::is_same<TComponent, MeshRendererComponent>())
							{
								const auto& sourceMesh = src.GetComponent<MeshRendererComponent>().Mesh;
								const auto& destinationMesh = dst.GetComponent<MeshRendererComponent>().Mesh;
								Material::Copy(destinationMesh->GetMaterial(), sourceMesh->GetMaterial());
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

	SharedRef<Scene> Scene::Copy(SharedRef<Scene> source)
	{
		VX_PROFILE_FUNCTION();

		SharedRef<Scene> destination = CreateShared<Scene>();

		destination->m_ViewportWidth = source->m_ViewportWidth;
		destination->m_ViewportHeight = source->m_ViewportHeight;

		auto& srcSceneRegistry = source->m_Registry;
		auto& dstSceneRegistry = destination->m_Registry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entites in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto& e : idView)
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

	void Scene::CreateDefaultEntities(const SharedRef<Scene>& context)
	{
		// Starting Entities
		Entity startingCube = context->CreateEntity("Cube");
		startingCube.AddComponent<MeshRendererComponent>();

		Entity startingSkyLight = context->CreateEntity("Sky Light");
		LightSourceComponent& lightSource = startingSkyLight.AddComponent<LightSourceComponent>();
		lightSource.Type = LightType::Directional;
		startingSkyLight.GetTransform().SetRotationEuler({ Math::Deg2Rad(0.25f), Math::Deg2Rad(-1.0f), Math::Deg2Rad(-0.5f) });
		startingSkyLight.GetTransform().Translation = { 0.0f, 4.0f, 0.0f };

		Entity startingCamera = context->CreateEntity("Camera");
		startingCamera.AddComponent<AudioListenerComponent>();
		SceneCamera& camera = startingCamera.AddComponent<CameraComponent>().Camera;
		camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		TransformComponent& cameraTransform = startingCamera.GetTransform();
		cameraTransform.Translation = { -4.0f, 3.0f, 4.0f };
		cameraTransform.SetRotationEuler({ Math::Deg2Rad(-25.0f), Math::Deg2Rad(-45.0f), 0.0f });
	}

	Entity Scene::CreateEntity(const std::string& name, const std::string& marker)
	{
		return CreateEntityWithUUID(UUID(), name, marker);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const std::string& marker)
	{
		VX_PROFILE_FUNCTION();

		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();

		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;
		tag.Marker = marker.empty() ? "UnTagged" : marker;

		entity.AddComponent<HierarchyComponent>();

		// Store the entity's UUID and the entt handle in our Entity map
		// entity here will be implicitly converted to an entt handle
		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity, bool excludeChildren)
	{
		VX_PROFILE_FUNCTION();

		// Call the entitys OnDestroy function if they are a script instance
		if (entity.HasComponent<ScriptComponent>() && ScriptEngine::GetContextScene() != nullptr)
		{
			const std::string& className = entity.GetComponent<ScriptComponent>().ClassName;

			if (ScriptEngine::EntityClassExists(className))
				ScriptEngine::OnDestroyEntity(entity);
		}

		if (entity.HasComponent<RigidBodyComponent>())
			Physics::DestroyPhysicsBody(entity);

		if (entity.HasComponent<RigidBody2DComponent>())
			Physics2D::DestroyPhysicsBody(entity);

		if (!excludeChildren)
		{
			for (size_t i = 0; i < entity.Children().size(); i++)
			{
				auto childID = entity.Children()[i];
				Entity child = TryGetEntityWithUUID(childID);
				DestroyEntity(child, excludeChildren);
			}
		}

		auto it = m_EntityMap.find(entity.GetUUID());
		m_Registry.destroy(entity);

		VX_CORE_ASSERT(it != m_EntityMap.end(), "Enitiy was not found in Entity Map!");

		// Remove the entity from our internal map
		m_EntityMap.erase(it->first);

		SortEntities();
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
				UnparentEntity(entity);
		}

		entity.SetParent(parent.GetUUID());
		parent.Children().push_back(entity.GetUUID());

		ConvertToLocalSpace(entity);
	}

	void Scene::UnparentEntity(Entity entity, bool convertToWorldSpace)
	{
		VX_PROFILE_FUNCTION();

		Entity parent = TryGetEntityWithUUID(entity.GetParentUUID());
		if (!parent)
			return;

		auto& parentChildren = parent.Children();
		parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), entity.GetUUID()), parentChildren.end());

		if (convertToWorldSpace)
			ConvertToWorldSpace(entity);

		entity.SetParent(0);
	}

    void Scene::ActiveateChildren(Entity entity)
    {
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

	void Scene::OnRuntimeStart(bool muteAudio)
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = true;
		m_DebugMode = false;

		OnPhysicsSimulationStart();

		// Audio Source - PlayOnStart
		if (!muteAudio)
		{
			auto view = m_Registry.view<AudioSourceComponent>();

			for (auto& e : view)
			{
				Entity entity{ e, this };

				if (!entity.IsActive())
					continue;

				SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
				const auto& audioProps = audioSource->GetProperties();

				if (audioProps.PlayOnStart)
				{
					audioSource->Play();
				}
			}
		}

		// Create Script Instances
		{
			ScriptEngine::OnRuntimeStart(this);

			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				ScriptEngine::OnCreateEntity(entity);
			}
		}
	}

	void Scene::OnRuntimeStop()
	{
		VX_PROFILE_FUNCTION();

		m_IsRunning = false;

		m_Registry.each([scene = this](auto& entityID)
		{
			Entity entity{ entityID, scene };
			if (entity.HasComponent<ScriptComponent>())
				ScriptEngine::OnDestroyEntity(entity);
		});

		ScriptEngine::OnRuntimeStop();

		// Stop all active audio sources in the scene
		{
			auto view = m_Registry.view<AudioSourceComponent>();

			for (auto& e : view)
			{
				Entity entity{ e, this };
				SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;

				if (!audioSource->IsPlaying())
					continue;

				audioSource->Stop();
			}
		}

		// Stop all active animators in the scene
		{
			auto view = m_Registry.view<AnimatorComponent>();

			for (auto& e : view)
			{
				Entity entity{ e, this };
				SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;

				if (!animator->IsPlaying())
					continue;

				animator->Stop();
			}
		}

		// Stop all active particle emitters in the scene
		{
			auto view = m_Registry.view<ParticleEmitterComponent>();

			for (auto& e : view)
			{
				Entity entity{ e, this };
				SharedRef<ParticleEmitter> particleEmitter = entity.GetComponent<ParticleEmitterComponent>().Emitter;

				if (!particleEmitter->IsActive())
					continue;

				particleEmitter->Stop();
			}
		}

		OnPhysicsSimulationStop();
	}

	void Scene::OnPhysicsSimulationStart()
	{
		VX_PROFILE_FUNCTION();

		Physics::OnSimulationStart(this);
		Physics2D::OnSimulationStart(this);
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

		if (!m_IsPaused || m_StepFrames > 0)
		{
			// Update Scripts
			if (m_ShouldUpdateScripts)
			{
				// C# Entity OnUpdate
				auto view = m_Registry.view<ScriptComponent>();
				for (auto entityID : view)
				{
					Entity entity{ entityID, this };

					if (!entity.IsActive())
						continue;

					ScriptEngine::OnUpdateEntity(entity, delta);
				}

				// C++ Entity OnUpdate
				m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
				{
					// TODO: Move to Scene::OnScenePlay
					if (!nsc.Instance)
					{
						nsc.Instance = nsc.InstantiateScript();
						nsc.Instance->m_Entity = Entity{ entity, this };
						nsc.Instance->OnCreate();
					}

					nsc.Instance->OnUpdate(delta);
				});
			}

			// Update Physics Bodies
			Physics::OnSimulationUpdate(delta, this);
			Physics2D::OnSimulationUpdate(delta, this);

			// Update Animators
			OnAnimatorUpdate(delta);

			if (m_StepFrames)
				m_StepFrames--;
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
			}

			if (primarySceneCamera != nullptr)
			{
				SceneRenderPacket renderPacket{};
				renderPacket.MainCamera = primarySceneCamera;
				renderPacket.Scene = this;
				renderPacket.CameraWorldSpaceTransform = primarySceneCameraTransform;
				renderPacket.EditorScene = false;
				s_SceneRenderer.RenderScene(renderPacket);
			}
		}

		// Update Components
		OnModelUpdate();
		OnParticleEmitterUpdate(delta);
	}

	void Scene::OnUpdateSimulation(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		if (!m_IsPaused || m_StepFrames > 0)
		{
			Physics::OnSimulationUpdate(delta, this);
			Physics2D::OnSimulationUpdate(delta, this);

			// Update Animators
			OnAnimatorUpdate(delta);

			if (m_StepFrames)
				m_StepFrames--;
		}

		// Render
		SceneRenderPacket renderPacket{};
		renderPacket.CameraWorldSpaceTransform = TransformComponent{};
		renderPacket.MainCamera = camera;
		renderPacket.Scene = this;
		renderPacket.EditorScene = true;
		s_SceneRenderer.RenderScene(renderPacket);

		// Update Components
		OnModelUpdate();
		OnParticleEmitterUpdate(delta);
	}

	void Scene::OnUpdateEditor(TimeStep delta, EditorCamera* camera)
	{
		VX_PROFILE_FUNCTION();

		// Update Animators
		OnAnimatorUpdate(delta);

		// Render
		SceneRenderPacket renderPacket{};
		renderPacket.CameraWorldSpaceTransform = TransformComponent{};
		renderPacket.MainCamera = camera;
		renderPacket.Scene = this;
		renderPacket.EditorScene = true;

		s_SceneRenderer.RenderScene(renderPacket);

		Entity primaryCameraEntity = GetPrimaryCameraEntity();
		if (primaryCameraEntity)
		{
			const auto& cameraComponent = primaryCameraEntity.GetComponent<CameraComponent>();

			// Set Clear color
			RenderCommand::SetClearColor(cameraComponent.ClearColor);
		}

		// Update Components
		OnModelUpdate();
		OnParticleEmitterUpdate(delta);
	}

	void Scene::OnUpdateEntityGui()
	{
		/*m_Registry.each([scene = this](auto& entityID)
		{
			Entity entity{ entityID, scene };
			if (entity.HasComponent<ScriptComponent>())
				ScriptEngine::OnGuiEntity(entity);
		});*/
	}

	void Scene::Step(uint32_t frames)
	{
		m_StepFrames = frames;
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

		for (auto& entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);

			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	Entity Scene::TryGetTopEntityInHierarchy(Entity child) const
	{
		if (!child.HasParent())
			return child;

		Entity parent = child.GetParent();
		return TryGetTopEntityInHierarchy(parent);
	}

	Entity Scene::DuplicateEntity(Entity src)
	{
		VX_PROFILE_FUNCTION();

		auto ParentNewEntityFunc = [&src](Entity newEntity)
		{
			if (auto parent = src.GetParent(); parent)
			{
				newEntity.SetParent(parent.GetUUID());
				parent.AddChild(newEntity.GetUUID());
			}
		};

		// TODO: handle prefabs
		/*if (src.HasComponent<PrefabComponent>())
		{
			auto prefabID = src.GetComponent<PrefabComponent>().PrefabUUID;
		}*/

		std::string name = src.GetName();
		std::string marker = src.GetMarker();
		Entity newEntity = CreateEntity(name, marker);

		// Copy components (except IDComponent and TagComponent)
		Utils::CopyComponentIfExists(AllComponents{}, newEntity, src);

		// We need to make a copy here because we modify it below
		auto childIDs = src.Children();

		for (auto childID : childIDs)
		{
			Entity childDuplicate = DuplicateEntity(TryGetEntityWithUUID(childID));

			// at this point childDuplicate is a child of src, we need to remove it from src
			UnparentEntity(childDuplicate);

			childDuplicate.SetParent(newEntity.GetUUID());
			newEntity.AddChild(childDuplicate.GetUUID());
		}

		ParentNewEntityFunc(newEntity);

		// TODO Duplicate script instance

		return newEntity;
	}

	Entity Scene::TryGetEntityWithUUID(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();

		for (auto entity : view)
		{
			const auto& idComponent = m_Registry.get<IDComponent>(entity);
			if (idComponent.ID == uuid)
				return Entity{ entity, this };
		}

		return Entity{};
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();

		for (auto& entity : view)
		{
			const auto& tag = view.get<TagComponent>(entity).Tag;
			if (strcmp(name.data(), tag.c_str()) == 0)
				return Entity{ entity, this };
		}

		return Entity{};
	}

	void Scene::ConvertToLocalSpace(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		Entity parent = TryGetEntityWithUUID(entity.GetParentUUID());

		if (!parent)
			return;

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
			return;

		Math::mat4 transform = GetWorldSpaceTransformMatrix(entity);
		auto& entityTransform = entity.GetTransform();
		entityTransform.SetTransform(transform);
	}

	Math::mat4 Scene::GetWorldSpaceTransformMatrix(Entity entity)
	{
		VX_PROFILE_FUNCTION();

		Math::mat4 transform(1.0f);

		Entity parent = TryGetEntityWithUUID(entity.GetParentUUID());

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

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();

		for (auto& entity : view)
		{
			auto& cc = view.get<CameraComponent>(entity);

			if (cc.Primary)
				return { entity, this };
		}

		return Entity{};
	}

	void Scene::SortEntities()
	{
		m_Registry.sort<IDComponent>([&](const auto lhs, const auto rhs)
		{
			auto lhsEntity = m_EntityMap.find(lhs.ID);
			auto rhsEntity = m_EntityMap.find(rhs.ID);
			return static_cast<uint32_t>(lhsEntity->second) < static_cast<uint32_t>(rhsEntity->second);
		});
	}

	void Scene::OnModelUpdate()
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<MeshRendererComponent>();

		for (auto& entity : view)
		{
			const auto& meshRendererComponent = view.get<MeshRendererComponent>(entity);

			meshRendererComponent.Mesh->OnUpdate((int)entity, meshRendererComponent.Scale);
		}
	}

	void Scene::OnAnimatorUpdate(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<AnimatorComponent, AnimationComponent, MeshRendererComponent>();

		for (auto& e : view)
		{
			Entity entity{ e, this };
			SharedRef<Animator> animator = entity.GetComponent<AnimatorComponent>().Animator;

			if (!animator || !animator->IsPlaying())
				continue;

			animator->UpdateAnimation(delta);
		}
	}

	void Scene::OnParticleEmitterUpdate(TimeStep delta)
	{
		VX_PROFILE_FUNCTION();

		auto view = m_Registry.view<ParticleEmitterComponent>();

		for (auto& e : view)
		{
			Entity entity{ e, this };
			SharedRef<ParticleEmitter> particleEmitter = entity.GetComponent<ParticleEmitterComponent>().Emitter;

			// Set the starting particle position to the entity's translation
			particleEmitter->GetProperties().Position = GetWorldSpaceTransform(entity).Translation;

			particleEmitter->OnUpdate(delta);

			if (particleEmitter->IsActive())
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

		Renderer::CreateShadowMap(component.Type);
	}

	template <> void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
		Model::Default defaultModel;
		ModelImportOptions importOptions = ModelImportOptions();

		if (component.Type == MeshType::Custom)
			defaultModel = Model::Default::Cube;
		else if (component.Type == MeshType::Capsule)
		{
			ModelImportOptions importOptions = ModelImportOptions();
			importOptions.MeshTransformation.SetRotationEuler({ 0.0f, 0.0f, 90.0f });
		}
		else
			defaultModel = (Model::Default)component.Type;

		component.Mesh = Model::Create(defaultModel, entity.GetTransform(), importOptions, (int)(entt::entity)entity);
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
			SharedRef<Model> model = entity.GetComponent<MeshRendererComponent>().Mesh;
			std::string filepath = model->GetPath();
			component.Animation = Animation::Create(filepath, model);
		}
	}

	template <> void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component) { }

	template <> void Scene::OnComponentAdded<AudioListenerComponent>(Entity entity, AudioListenerComponent& component)
	{
		component.Listener = CreateShared<AudioListener>();
		AudioSource::AddAudioListener();
	}
	
	template <> void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component) { }

	template <> void Scene::OnComponentAdded<PhysicsMaterialComponent>(Entity entity, PhysicsMaterialComponent& component) { }

	template <> void Scene::OnComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<StaticMeshColliderComponent>(Entity entity, StaticMeshColliderComponent& component) { }

	template <> void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component) { }

	template <> void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<NavMeshAgentComponent>(Entity entity, NavMeshAgentComponent& component) { }

	template <> void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) { }

	template <> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

	SharedRef<Scene> Scene::Create()
	{
		return CreateShared<Scene>();
	}

}

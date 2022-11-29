#include "sppch.h"
#include "Scene.h"

#include "Sparky/Core/Math.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Audio/AudioEngine.h"
#include "Sparky/Renderer/Renderer.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Renderer/LightSource.h"
#include "Sparky/Renderer/ParticleEmitter.h"
#include "Sparky/Scene/ScriptableEntity.h"
#include "Sparky/Scene/Prefab.h"
#include "Sparky/Scripting/ScriptEngine.h"
#include "Sparky/Asset/AssetRegistry.h"
#include "Sparky/Scene/SceneRenderer.h"
#include "Sparky/Renderer/Font/Font.h"

#include "Sparky/Physics/Physics.h"
#include "Sparky/Physics/Physics2D.h"

namespace Sparky {

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
							if (std::is_same<TComponent, MeshRendererComponent>())
							{
								const auto& sourceMesh = src.GetComponent<MeshRendererComponent>().Mesh;
								const auto& destinationMesh = dst.GetComponent<MeshRendererComponent>().Mesh;
								Material::Copy(destinationMesh->GetMaterial(), sourceMesh->GetMaterial());
							}

							if (std::is_same<TComponent, LightSourceComponent>())
							{
								const auto& sourceLightSource = src.GetComponent<LightSourceComponent>().Source;
								const auto& destinationLightSource = dst.GetComponent<LightSourceComponent>().Source;
								LightSource::Copy(destinationLightSource, sourceLightSource);
							}

							if (std::is_same<TComponent, AudioSourceComponent>())
							{
								const auto& sourceAudioSource = src.GetComponent<AudioSourceComponent>().Source;
								const auto& destinationAudioSource = dst.GetComponent<AudioSourceComponent>().Source;
								AudioSource::Copy(destinationAudioSource, sourceAudioSource);
							}

							if (std::is_same<TComponent, ParticleEmitterComponent>())
							{
								const auto& sourceEmitter = src.GetComponent<ParticleEmitterComponent>().Emitter;
								const auto& destinationEmitter = dst.GetComponent<ParticleEmitterComponent>().Emitter;
								ParticleEmitter::Copy(destinationEmitter, sourceEmitter);
							}

							// If we copy a script component, we should probably copy all of the script field values as well
							if (std::is_same<TComponent, ScriptComponent>())
							{
								const auto& sourceScriptFieldMap = ScriptEngine::GetScriptFieldMap(src);
								auto& destinationScriptFieldMap = ScriptEngine::GetScriptFieldMap(dst);

								for (const auto& [name, field] : sourceScriptFieldMap)
									destinationScriptFieldMap[name] = field;
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
		SP_PROFILE_FUNCTION();

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

		return destination;
	}

	void Scene::CreateDefaultEntities(const SharedRef<Scene>& context)
	{
		// Starting Entities
		Entity startingCube = context->CreateEntity("Cube");
		startingCube.AddComponent<MeshRendererComponent>();

		Entity startingPointLight = context->CreateEntity("Point Light");
		startingPointLight.AddComponent<LightSourceComponent>().Type = LightSourceComponent::LightType::Point;
		startingPointLight.GetTransform().Translation = Math::vec3(-2.0f, 4.0f, 4.0f);

		Entity startingCamera = context->CreateEntity("Camera");
		startingCamera.AddComponent<AudioListenerComponent>();
		SceneCamera& camera = startingCamera.AddComponent<CameraComponent>().Camera;
		camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
		TransformComponent& cameraTransform = startingCamera.GetTransform();
		cameraTransform.Translation = Math::vec3(-4.0f, 3.0f, 4.0f);
		cameraTransform.SetRotationEuler(Math::vec3(Math::Deg2Rad(-25.0f), Math::Deg2Rad(-45.0f), 0.0f));
	}

	Entity Scene::CreateEntity(const std::string& name, const std::string& marker)
	{
		return CreateEntityWithUUID(UUID(), name, marker);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name, const std::string& marker)
	{
		SP_PROFILE_FUNCTION();

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

	void Scene::DestroyEntity(Entity entity, bool isEntityInstance, bool excludeChildren)
	{
		SP_PROFILE_FUNCTION();

		// Call the entitys OnDestroy function if they are a script instance
		if (isEntityInstance)
			ScriptEngine::OnDestroyEntity(entity);

		if (entity.HasComponent<RigidBodyComponent>())
			Physics::DestroyPhysicsBody(entity);

		if (entity.HasComponent<RigidBody2DComponent>())
			Physics2D::DestroyPhysicsBody(entity);

		if (!excludeChildren)
		{
			for (size_t i = 0; i < entity.Children().size(); i++)
			{
				const auto& childID = entity.Children()[i];
				Entity child = TryGetEntityWithUUID(childID);
				DestroyEntity(child, isEntityInstance, excludeChildren);
			}
		}

		auto it = m_EntityMap.find(entity.GetUUID());
		m_Registry.destroy(entity);

		SP_CORE_ASSERT(it != m_EntityMap.end(), "Enitiy was not found in Entity Map!");

		// Remove the entity from our internal map
		m_EntityMap.erase(it->first);
	}

	void Scene::ParentEntity(Entity entity, Entity parent)
	{
		SP_PROFILE_FUNCTION();

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
		SP_PROFILE_FUNCTION();

		Entity parent = TryGetEntityWithUUID(entity.GetParentUUID());
		if (!parent)
			return;

		auto& parentChildren = parent.Children();
		parentChildren.erase(std::remove(parentChildren.begin(), parentChildren.end(), entity.GetUUID()), parentChildren.end());

		if (convertToWorldSpace)
			ConvertToWorldSpace(entity);

		entity.SetParent(0);
	}

	void Scene::OnRuntimeStart()
	{
		SP_PROFILE_FUNCTION();

		m_IsRunning = true;
		m_DebugMode = false;

		OnPhysicsSimulationStart();

		{
			auto view = m_Registry.view<AudioSourceComponent>();

			for (auto& e : view)
			{
				Entity entity{ e, this };
				SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;
				const auto& audioProps = audioSource->GetProperties();

				if (audioProps.PlayOnStart)
				{
					audioSource->Play();
				}
			}
		}

		// Scripting
		{
			ScriptEngine::OnRuntimeStart(this);

			// Instantiate all script entities
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
		SP_PROFILE_FUNCTION();

		m_IsRunning = false;

		ScriptEngine::OnRuntimeStop();

		// Stop all active audio sources in the scene
		{
			auto view = m_Registry.view<AudioSourceComponent>();

			for (auto& e : view)
			{
				Entity entity{ e, this };
				SharedRef<AudioSource> audioSource = entity.GetComponent<AudioSourceComponent>().Source;

				if (audioSource->IsPlaying())
					audioSource->Stop();
			}
		}

		// Stop all active particle emitters in the scene
		{
			auto view = m_Registry.view<ParticleEmitterComponent>();

			for (auto& e : view)
			{
				Entity entity{ e, this };
				SharedRef<ParticleEmitter> particleEmitter = entity.GetComponent<ParticleEmitterComponent>().Emitter;

				if (particleEmitter->IsActive())
					particleEmitter->Stop();
			}
		}

		OnPhysicsSimulationStop();
	}

	void Scene::OnPhysicsSimulationStart()
	{
		SP_PROFILE_FUNCTION();

		Physics::OnSimulationStart(this);
		Physics2D::OnSimulationStart(this);
	}

	void Scene::OnPhysicsSimulationStop()
	{
		SP_PROFILE_FUNCTION();

		Physics::OnSimulationStop();
		Physics2D::OnSimulationStop();
	}

	void Scene::OnUpdateRuntime(TimeStep delta)
	{
		SP_PROFILE_FUNCTION();

		if (!m_IsPaused || m_StepFrames > 0)
		{
			// Update Scripts
			{
				// C# Entity OnUpdate
				auto view = m_Registry.view<ScriptComponent>();
				for (auto entityID : view)
					ScriptEngine::OnUpdateEntity(Entity{ entityID, this }, delta);

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

			// Render from primary scene camera's point of view
			if (primarySceneCamera != nullptr)
			{
				s_SceneRenderer.RenderFromSceneCamera(*primarySceneCamera, primarySceneCameraTransform, this);
			}
		}

		// Update Components
		OnModelUpdate();
		OnParticleEmitterUpdate(delta);
		OnLightSourceUpdate();
	}

	void Scene::OnUpdateSimulation(TimeStep delta, EditorCamera& camera)
	{
		SP_PROFILE_FUNCTION();

		if (!m_IsPaused || m_StepFrames > 0)
		{
			Physics::OnSimulationUpdate(delta, this);
			Physics2D::OnSimulationUpdate(delta, this);

			if (m_StepFrames)
				m_StepFrames--;
		}

		// Render
		s_SceneRenderer.RenderFromEditorCamera(camera, this);

		// Update Components
		OnModelUpdate();
		OnParticleEmitterUpdate(delta);
		OnLightSourceUpdate();
	}

	void Scene::OnUpdateEditor(TimeStep delta, EditorCamera& camera)
	{
		SP_PROFILE_FUNCTION();

		// Render
		s_SceneRenderer.RenderFromEditorCamera(camera, this);

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
		OnLightSourceUpdate();
	}

	void Scene::OnUpdateEntityGui()
	{
		
	}

	void Scene::Step(uint32_t frames)
	{
		m_StepFrames = frames;
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		SP_PROFILE_FUNCTION();

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

	Entity Scene::DuplicateEntity(Entity src)
	{
		SP_PROFILE_FUNCTION();

		std::string name = src.GetName();
		std::string marker = src.GetMarker();
		Entity dest = CreateEntity(name, marker);

		// TODO: handle prefabs
		/*if (src.HasComponent<PrefabComponent>())
		{

		}*/

		// Copy components (except IDComponent and TagComponent)
		Utils::CopyComponentIfExists(AllComponents{}, dest, src);

		if (src.HasParent())
			dest.SetParent(src.GetParentUUID());

		const auto& srcChildren = src.Children();
		for (auto& childID : srcChildren)
		{
			Entity childEntity = TryGetEntityWithUUID(childID);
			SP_CORE_ASSERT(childEntity, "Child ID was Invalid! --- Should never happen");
			Entity childDuplicate = DuplicateEntity(childEntity);

			childDuplicate.SetParent(childEntity.GetParentUUID());
			dest.Children().push_back(childDuplicate.GetUUID());
		}

		return dest;
	}

	Entity Scene::TryGetEntityWithUUID(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();

		for (auto entity : view)
		{
			auto& idComponent = m_Registry.get<IDComponent>(entity);
			if (idComponent.ID == uuid)
				return Entity(entity, this);
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
		SP_PROFILE_FUNCTION();

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
		SP_PROFILE_FUNCTION();

		Entity parent = TryGetEntityWithUUID(entity.GetParentUUID());

		if (!parent)
			return;

		Math::mat4 transform = GetWorldSpaceTransformMatrix(entity);
		auto& entityTransform = entity.GetTransform();
		entityTransform.SetTransform(transform);
	}

	Math::mat4 Scene::GetWorldSpaceTransformMatrix(Entity entity)
	{
		SP_PROFILE_FUNCTION();

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
		SP_PROFILE_FUNCTION();

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

	void Scene::OnModelUpdate()
	{
		SP_PROFILE_FUNCTION();

		auto view = m_Registry.view<MeshRendererComponent>();

		for (auto& entity : view)
		{
			auto& meshRendererComponent = view.get<MeshRendererComponent>(entity);

			meshRendererComponent.Mesh->OnUpdate((int)entity, meshRendererComponent.Scale);
		}
	}

	void Scene::OnParticleEmitterUpdate(TimeStep delta)
	{
		SP_PROFILE_FUNCTION();

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

	void Scene::OnLightSourceUpdate()
	{
		SP_PROFILE_FUNCTION();

		auto view = m_Registry.view<LightSourceComponent>();

		for (auto& e : view)
		{
			Entity entity{ e, this };
			const LightSourceComponent& lightSourceComponent = entity.GetComponent<LightSourceComponent>();
			SharedRef<LightSource> lightSource = lightSourceComponent.Source;
			
			Math::vec3 position = GetWorldSpaceTransform(entity).Translation;
			lightSource->SetPosition(position);
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

	template <> void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
		component.Mesh = Model::Create(Model::Default::Cube, entity.GetTransform(), (int)(entt::entity)entity);
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
	
	template <> void Scene::OnComponentAdded<AudioSourceComponent>(Entity entity, AudioSourceComponent& component)
	{
		
	}

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

	template <> void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) { }

	template <> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

	SharedRef<Scene> Scene::Create()
	{
		return CreateShared<Scene>();
	}
}

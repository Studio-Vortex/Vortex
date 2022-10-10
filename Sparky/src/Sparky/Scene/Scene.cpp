#include "sppch.h"
#include "Scene.h"

#include "Sparky/Scene/ScriptableEntity.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Core/Math.h"
#include "Sparky/Scripting/ScriptEngine.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_polygon_shape.h>

namespace Sparky {

	static b2BodyType RigidBody2DTypeToBox2DBody(RigidBody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case Sparky::RigidBody2DComponent::BodyType::Static:    return b2_staticBody;
			case Sparky::RigidBody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
			case Sparky::RigidBody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}

		SP_CORE_ASSERT(false, "Unknown body type!");
		return b2_staticBody;
	}

	Scene::Scene() { }

	Scene::~Scene()
	{
		delete m_PhysicsWorld;
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template <typename... TComponents>
	static void CopyComponent(entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
		{
			auto view = src.view<TComponents>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

				auto& srcComponent = src.get<TComponents>(srcEntity);
				dst.emplace_or_replace<TComponents>(dstEntity, srcComponent);
			}
		}(), ...);
	}

	template<typename... TComponents>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
		{
			if (src.HasComponent<TComponents>())
				dst.AddOrReplaceComponent<TComponents>(src.GetComponent<TComponents>());
		}(), ...);
	}

	template<typename... TComponents>
	static void CopyComponentIfExists(ComponentGroup<TComponents...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<TComponents...>(dst, src);
	}

	SharedRef<Scene> Scene::Copy(SharedRef<Scene> source)
	{
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
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return destination;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		// Store the entity's UUID and the entt handle in our Entity map
		// entity here will be implicitly converted to an entt handle
		m_EntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity, bool isEntityInstance)
	{
		if (isEntityInstance)
		{
			// Call the entitys OnDestroy function
			ScriptEngine::OnDestroyEntity(entity);
		}

		// Destroy the physics body and fixture if they exist
		if (entity.HasComponent<RigidBody2DComponent>())
		{
			b2Body* body = (b2Body*)entity.GetComponent<RigidBody2DComponent>().RuntimeBody;
			
			if (body != nullptr)
			{
				if (entity.HasComponent<BoxCollider2DComponent>())
				{
					b2Fixture* fixture = (b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture;

					// Remove the fixture and release the stored user data
					auto it = m_PhysicsBodyDataMap.find(fixture);
					SP_CORE_ASSERT(it != m_PhysicsBodyDataMap.end(), "Physics body was not found in Physics Body Data Map!");
					m_PhysicsBodyDataMap.erase(it->first);

					if (fixture != nullptr)
						body->DestroyFixture(fixture);
				}

				if (entity.HasComponent<CircleCollider2DComponent>())
				{
					b2Fixture* fixture = (b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture;

					// Remove the fixture and release the stored user data
					auto it = m_PhysicsBodyDataMap.find(fixture);
					SP_CORE_ASSERT(it != m_PhysicsBodyDataMap.end(), "Physics body was not found in Physics Body Data Map!");
					m_PhysicsBodyDataMap.erase(it->first);

					if (fixture != nullptr)
						body->DestroyFixture(fixture);
				}

				m_PhysicsWorld->DestroyBody(body);
			}
		}

		auto it = m_EntityMap.find(entity.GetUUID());
		m_Registry.destroy(entity);

		SP_CORE_ASSERT(it != m_EntityMap.end(), "Enitiy was not found in Entity Map!");

		// Remove the entity from our internal map
		m_EntityMap.erase(it->first);
	}

	void Scene::OnRuntimeStart()
	{
		m_IsRunning = true;

		OnPhysics2DStart();

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
		m_IsRunning = false;

		ScriptEngine::OnRuntimeStop();

		OnPhysics2DStop();
	}

	void Scene::OnSimulationStart()
	{
		OnPhysics2DStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysics2DStop();
	}

	void Scene::OnUpdateRuntime(TimeStep delta)
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

		OnPhysics2DUpdate(delta);

		// Render 2D
		Camera* mainCamera = nullptr;
		Math::mat4 cameraTransform;

		{
			auto view = m_Registry.view<TransformComponent, CameraComponent>();

			for (auto entity : view)
			{
				auto [transformComponent, cameraComponent] = view.get<TransformComponent, CameraComponent>(entity);

				if (cameraComponent.Primary)
				{
					mainCamera = &cameraComponent.Camera;
					cameraTransform = transformComponent.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			/// Render Sprites
			{
				auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
				for (auto entity : group)
				{
					auto [transformComponent, spriteComponent] = group.get<TransformComponent, SpriteRendererComponent>(entity);

					Renderer2D::DrawSprite(transformComponent.GetTransform(), spriteComponent, (int)entity);
				}
			}

			/// Render Circles
			{
				auto view = m_Registry.view<TransformComponent, CircleRendererComponent>();
				for (auto entity : view)
				{
					auto [transformComponent, circle] = view.get<TransformComponent, CircleRendererComponent>(entity);

					Renderer2D::DrawCircle(transformComponent.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
				}
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateSimulation(TimeStep delta, EditorCamera& camera)
	{
		OnPhysics2DUpdate(delta);

		// Render
		RenderScene(camera);
	}

	void Scene::OnUpdateEditor(TimeStep delta, EditorCamera& camera)
	{
		// Render
		RenderScene(camera);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
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
		std::string name = src.GetName();
		name.append(" Copy"); // Allow the user to tell the difference between original entity and copy
		Entity dest = CreateEntity(name);

		// Copy components (except IDComponent and TagComponent)
		CopyComponentIfExists(AllComponents{}, dest, src);

		return dest;
	}

	Entity Scene::GetEntityWithUUID(UUID uuid)
	{
		SP_CORE_ASSERT(m_EntityMap.find(uuid) != m_EntityMap.end(), "UUID was not present in Entity Map!");

		return Entity{ m_EntityMap.at(uuid), this};
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

	void Scene::OnPhysicsBodyCreate(Entity entity, const TransformComponent& transform, RigidBody2DComponent& rb2d)
	{
		b2BodyDef bodyDef;
		bodyDef.type = RigidBody2DTypeToBox2DBody(rb2d.Type);
		bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
		bodyDef.angle = transform.Rotation.z;

		b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
		body->SetFixedRotation(rb2d.FixedRotation);

		rb2d.RuntimeBody = body;

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

			b2PolygonShape boxShape;
			// Automatically set the collider size to the scale of the entity
			boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y, b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

			b2FixtureDef fixtureDef;

			UniqueRef<PhysicsBodyData> physicsBodyData = CreateUnique<PhysicsBodyData>();
			physicsBodyData->EntityUUID = entity.GetUUID();
			fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(physicsBodyData.get());

			fixtureDef.shape = &boxShape;
			fixtureDef.density = bc2d.Density;
			fixtureDef.friction = bc2d.Friction;
			fixtureDef.restitution = bc2d.Restitution;
			fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

			b2Fixture* fixture = body->CreateFixture(&fixtureDef);

			// Store the fixture and the user data in our map
			if (m_PhysicsBodyDataMap.find(fixture) == m_PhysicsBodyDataMap.end())
				m_PhysicsBodyDataMap[fixture] = std::move(physicsBodyData);
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

			b2CircleShape circleShape;
			circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
			circleShape.m_radius = transform.Scale.x * cc2d.Radius;

			b2FixtureDef fixtureDef;

			UniqueRef<PhysicsBodyData> physicsBodyData = CreateUnique<PhysicsBodyData>();
			physicsBodyData->EntityUUID = entity.GetUUID();
			fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(physicsBodyData.get());

			fixtureDef.shape = &circleShape;
			fixtureDef.density = cc2d.Density;
			fixtureDef.friction = cc2d.Friction;
			fixtureDef.restitution = cc2d.Restitution;
			fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;

			b2Fixture* fixture = body->CreateFixture(&fixtureDef);

			// Store the fixture and the user data in our map
			if (m_PhysicsBodyDataMap.find(fixture) == m_PhysicsBodyDataMap.end())
				m_PhysicsBodyDataMap[fixture] = std::move(physicsBodyData);
		}
	}

	void Scene::OnPhysics2DStart()
	{
		m_PhysicsWorld = new b2World({ s_PhysicsWorldGravity.x, s_PhysicsWorldGravity.y });

		auto view = m_Registry.view<RigidBody2DComponent>();

		for (auto e : view)
		{
			Entity entity{ e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

			OnPhysicsBodyCreate(entity, transform, rb2d);
		}
	}

	void Scene::OnPhysics2DUpdate(TimeStep delta)
	{
		m_PhysicsWorld->SetGravity({ s_PhysicsWorldGravity.x, s_PhysicsWorldGravity.y });

		// Physics
		{
			// Copies transform from Sparky to Box2D
			auto view = m_Registry.view<RigidBody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

				if (rb2d.RuntimeBody == nullptr)
					OnPhysicsBodyCreate(entity, transform, rb2d);

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				glm::vec3 translation = transform.Translation;
				float angle = transform.Rotation.z;

				const auto& bodyPosition = body->GetPosition();
				const float bodyAngle = body->GetAngle();

				bool awake = bodyPosition.x != transform.Translation.x || bodyPosition.y != transform.Translation.y || bodyAngle != angle;

				body->SetTransform({ translation.x, translation.y }, angle);

				if (awake)
					body->SetAwake(true);
			}

			m_PhysicsWorld->Step(delta, s_PhysicsWorldVeloctityIterations, s_PhysicsWorldPositionIterations);

			// Get transform from Box2D
			for (auto e : view)
			{
				Entity entity{ e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

				// If a rb2d component is added during runtime we can create the physics body here
				if (rb2d.RuntimeBody == nullptr)
					OnPhysicsBodyCreate(entity, transform, rb2d);

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				const auto& position = body->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = body->GetAngle();
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
		m_PhysicsBodyDataMap.clear();
	}

	void Scene::RenderScene(EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		/// Render Sprites
		{
			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();

			for (auto entity : view)
			{
				auto [transformComponent, spriteComponent] = view.get<TransformComponent, SpriteRendererComponent>(entity);

				Renderer2D::DrawSprite(transformComponent.GetTransform(), spriteComponent, (int)entity);
			}
		}

		/// Render Circles
		{
			auto group = m_Registry.group<TransformComponent>(entt::get<CircleRendererComponent>);

			for (auto entity : group)
			{
				auto [transformComponent, circle] = group.get<TransformComponent, CircleRendererComponent>(entity);

				Renderer2D::DrawCircle(transformComponent.GetTransform(), circle.Color, circle.Thickness, circle.Fade, (int)entity);
			}
		}

		Renderer2D::EndScene();
	}

	template <typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) != 0);
	}

	template <> void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) { }
	
	template <> void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) { }

	template <> void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) { }
	
	template <> void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component) { }

	template <> void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) { }
	
	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	
	template <> void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component) { }

	template <> void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<ScriptComponent>(Entity entity, ScriptComponent& component) { }

	template <> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

}

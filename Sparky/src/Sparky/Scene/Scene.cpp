#include "sppch.h"
#include "Scene.h"

#include "Sparky/Scene/Components.h"
#include "Sparky/Scene/ScriptableEntity.h"
#include "Sparky/Scene/Entity.h"
#include "Sparky/Renderer/Renderer2D.h"
#include "Sparky/Core/Math.h"

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

	template <typename TComponent>
	static void CopyComponent(entt::registry& dst, const entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<TComponent>();

		for (auto& e : view)
		{
			UUID uuid = src.get<IDComponent>(e).ID;
			SP_CORE_ASSERT(enttMap.find(uuid) != enttMap.end(), "Entity's UUID was not found in enttMap!")
			entt::entity dstEnttID = enttMap.at(uuid);

			auto& component = src.get<TComponent>(e);
			dst.emplace_or_replace<TComponent>(dstEnttID, component);
		}
	}

	template <typename TComponent>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<TComponent>())
			dst.AddOrReplaceComponent<TComponent>(src.GetComponent<TComponent>());
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
		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<RigidBody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

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

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		OnPhysics2DStart();
	}

	void Scene::OnRuntimeStop()
	{
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

		// Physics
		{
			const int32_t velocityIterations = 6; // TODO: Expose in editor settings
			const int32_t positionIterations = 2; // TODO: Expose in editor settings
			m_PhysicsWorld->Step(delta, velocityIterations, positionIterations);

			// Get transform from Box2D
			auto view = m_Registry.view<RigidBody2DComponent>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				const auto& position = body->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = body->GetAngle();
			}
		}

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
				auto group = m_Registry.group<TransformComponent>(entt::get<SpriteComponent>);
				for (auto entity : group)
				{
					auto [transformComponent, spriteComponent] = group.get<TransformComponent, SpriteComponent>(entity);

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
		// Physics
		{
			const int32_t velocityIterations = 6; // TODO: Expose in editor settings
			const int32_t positionIterations = 2; // TODO: Expose in editor settings
			m_PhysicsWorld->Step(delta, velocityIterations, positionIterations);

			// Get transform from Box2D
			auto view = m_Registry.view<RigidBody2DComponent>();
			for (auto e : view)
			{
				Entity entity{ e, this };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				const auto& position = body->GetPosition();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = body->GetAngle();
			}
		}

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

	Entity Scene::DuplicateEntity(Entity source)
	{
		std::string name = source.GetName();
		name.append(" Copy"); // Allow the user to tell the difference between original entity and copy
		Entity duplicatedEntity = CreateEntity(name);

		// Copy components (except IDComponent and TagComponent)
		CopyComponentIfExists<TransformComponent>(duplicatedEntity, source);
		CopyComponentIfExists<SpriteComponent>(duplicatedEntity, source);
		CopyComponentIfExists<CircleRendererComponent>(duplicatedEntity, source);
		CopyComponentIfExists<CameraComponent>(duplicatedEntity, source);
		CopyComponentIfExists<NativeScriptComponent>(duplicatedEntity, source);
		CopyComponentIfExists<RigidBody2DComponent>(duplicatedEntity, source);
		CopyComponentIfExists<BoxCollider2DComponent>(duplicatedEntity, source);
		CopyComponentIfExists<CircleCollider2DComponent>(duplicatedEntity, source);

		return duplicatedEntity;
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

	void Scene::OnPhysics2DStart()
	{
		b2Vec2 gravity = { 0.0f, -9.8f };
		m_PhysicsWorld = new b2World(gravity);

		auto view = m_Registry.view<RigidBody2DComponent>();

		for (auto e : view)
		{
			Entity entity{ e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

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
				boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &boxShape;
				fixtureDef.density = bc2d.Density;
				fixtureDef.friction = bc2d.Friction;
				fixtureDef.restitution = bc2d.Restitution;
				fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

				b2CircleShape circleShape;
				circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
				circleShape.m_radius = transform.Scale.x * cc2d.Radius;

				b2FixtureDef fixtureDef;
				fixtureDef.shape = &circleShape;
				fixtureDef.density = cc2d.Density;
				fixtureDef.friction = cc2d.Friction;
				fixtureDef.restitution = cc2d.Restitution;
				fixtureDef.restitutionThreshold = cc2d.RestitutionThreshold;

				body->CreateFixture(&fixtureDef);
			}
		}
	}

	void Scene::OnPhysics2DStop()
	{
		delete m_PhysicsWorld;
		m_PhysicsWorld = nullptr;
	}

	void Scene::RenderScene(EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		/// Render Sprites
		{
			auto view = m_Registry.view<TransformComponent, SpriteComponent>();
			for (auto entity : view)
			{
				auto [transformComponent, spriteComponent] = view.get<TransformComponent, SpriteComponent>(entity);

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
		//SP_CORE_ASSERT(false, "Should not be calling base template function!");
	}

	template <> void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component) { }
	
	template <> void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component) { }

	template <> void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component) { }
	
	template <> void Scene::OnComponentAdded<SpriteComponent>(Entity entity, SpriteComponent& component) { }

	template <> void Scene::OnComponentAdded<CircleRendererComponent>(Entity entity, CircleRendererComponent& component) { }
	
	template <>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}
	
	template <> void Scene::OnComponentAdded<RigidBody2DComponent>(Entity entity, RigidBody2DComponent& component) { }

	template <> void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity entity, BoxCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity entity, CircleCollider2DComponent& component) { }

	template <> void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component) { }

}
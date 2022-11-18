#include "sppch.h"
#include "Physics2D.h"

#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_circle_shape.h>
#include <box2d/b2_polygon_shape.h>

namespace Sparky {

	namespace Utils {

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

	}

	void Physics2D::OnSimulationStart(Scene* contextScene)
	{
		s_PhysicsScene = new b2World({ s_PhysicsWorld2DGravity.x, s_PhysicsWorld2DGravity.y });

		auto view = contextScene->GetAllEntitiesWith<RigidBody2DComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };
			auto& transform = entity.GetTransform();
			auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

			CreatePhysicsBody(entity, transform, rb2d);
		}
	}

	void Physics2D::OnSimulationUpdate(TimeStep delta, Scene* contextScene)
	{
		s_PhysicsScene->SetGravity({ s_PhysicsWorld2DGravity.x, s_PhysicsWorld2DGravity.y });

		// Physics
		{
			// Copies transform from Sparky to Box2D
			auto view = contextScene->GetAllEntitiesWith<RigidBody2DComponent>();

			for (auto e : view)
			{
				Entity entity = { e, contextScene };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

				// If a rb2d component is added during runtime we can create the physics body here
				if (rb2d.RuntimeBody == nullptr)
					CreatePhysicsBody(entity, transform, rb2d);

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				Math::vec3 translation = transform.Translation;
				float angle = transform.Rotation.z;

				const auto& bodyPosition = body->GetPosition();
				const float bodyAngle = body->GetAngle();

				bool awake = bodyPosition.x != translation.x || bodyPosition.y != translation.y || bodyAngle != angle;

				body->SetTransform({ translation.x, translation.y }, angle);
				if (rb2d.Velocity != Math::vec2(0.0f))
					body->SetLinearVelocity({ rb2d.Velocity.x, rb2d.Velocity.y });
				body->SetLinearDamping(rb2d.Drag);
				body->SetFixedRotation(rb2d.FixedRotation);

				if (entity.HasComponent<BoxCollider2DComponent>())
				{
					auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
					b2Fixture* fixture = (b2Fixture*)bc2d.RuntimeFixture;

					fixture->SetDensity(bc2d.Density);
					fixture->SetFriction(bc2d.Friction);
					fixture->SetRestitution(bc2d.Restitution);
					fixture->SetRestitutionThreshold(bc2d.RestitutionThreshold);
					fixture->SetSensor(bc2d.IsTrigger);
				}

				if (entity.HasComponent<CircleCollider2DComponent>())
				{
					auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
					b2Fixture* fixture = (b2Fixture*)cc2d.RuntimeFixture;

					fixture->SetDensity(cc2d.Density);
					fixture->SetFriction(cc2d.Friction);
					fixture->SetRestitution(cc2d.Restitution);
					fixture->SetRestitutionThreshold(cc2d.RestitutionThreshold);
				}

				if (awake)
					body->SetAwake(true);
			}

			s_PhysicsScene->Step(delta, s_PhysicsWorld2DVeloctityIterations, s_PhysicsWorld2DPositionIterations);

			// Get transform from Box2D
			for (auto e : view)
			{
				Entity entity{ e, contextScene };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rb2d = entity.GetComponent<RigidBody2DComponent>();

				b2Body* body = (b2Body*)rb2d.RuntimeBody;
				const auto& position = body->GetPosition();
				transform.Translation = Math::vec3(position.x, position.y, transform.Translation.z);
				transform.Rotation.z = body->GetAngle();
			}
		}
	}

	void Physics2D::OnSimulationStop()
	{
		delete s_PhysicsScene;
		s_PhysicsScene = nullptr;
		s_PhysicsBodyDataMap.clear();
	}

	void Physics2D::CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBody2DComponent& rb2d)
	{
		b2BodyDef bodyDef;
		bodyDef.type = Utils::RigidBody2DTypeToBox2DBody(rb2d.Type);
		bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
		bodyDef.fixedRotation = rb2d.FixedRotation;
		bodyDef.linearVelocity = b2Vec2(rb2d.Velocity.x, rb2d.Velocity.y);
		bodyDef.linearDamping = rb2d.Drag;
		bodyDef.angle = transform.Rotation.z;

		b2Body* body = s_PhysicsScene->CreateBody(&bodyDef);

		rb2d.RuntimeBody = body;

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

			b2PolygonShape boxShape;
			// Automatically set the collider size to the scale of the entity
			boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y,	b2Vec2(bc2d.Offset.x, bc2d.Offset.y), 0.0f);

			b2FixtureDef fixtureDef;

			UniqueRef<PhysicsBodyData> physicsBodyData = CreateUnique<PhysicsBodyData>();
			physicsBodyData->EntityUUID = entity.GetUUID();
			fixtureDef.userData.pointer = reinterpret_cast<uintptr_t>(physicsBodyData.get());

			fixtureDef.shape = &boxShape;
			fixtureDef.isSensor = bc2d.IsTrigger;
			fixtureDef.density = bc2d.Density;
			fixtureDef.friction = bc2d.Friction;
			fixtureDef.restitution = bc2d.Restitution;
			fixtureDef.restitutionThreshold = bc2d.RestitutionThreshold;

			b2Fixture* fixture = body->CreateFixture(&fixtureDef);

			bc2d.RuntimeFixture = fixture;

			// Store the fixture and the user data in our map
			if (s_PhysicsBodyDataMap.find(fixture) == s_PhysicsBodyDataMap.end())
				s_PhysicsBodyDataMap[fixture] = std::move(physicsBodyData);
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

			cc2d.RuntimeFixture = fixture;

			// Store the fixture and the user data in our map
			if (s_PhysicsBodyDataMap.find(fixture) == s_PhysicsBodyDataMap.end())
				s_PhysicsBodyDataMap[fixture] = std::move(physicsBodyData);
		}
	}

	void Physics2D::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body and fixture if they exist

		b2Body* entityRuntimePhysicsBody = (b2Body*)entity.GetComponent<RigidBody2DComponent>().RuntimeBody;

		if (entityRuntimePhysicsBody != nullptr)
		{
			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				b2Fixture* fixture = (b2Fixture*)entity.GetComponent<BoxCollider2DComponent>().RuntimeFixture;

				// Remove the fixture and release the stored user data
				if (fixture != nullptr)
				{
					auto it = s_PhysicsBodyDataMap.find(fixture);
					SP_CORE_ASSERT(it != s_PhysicsBodyDataMap.end(), "Physics body was not found in Physics Body Data Map!");
					s_PhysicsBodyDataMap.erase(it->first);

					entityRuntimePhysicsBody->DestroyFixture(fixture);
				}
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				b2Fixture* fixture = (b2Fixture*)entity.GetComponent<CircleCollider2DComponent>().RuntimeFixture;

				// Remove the fixture and release the stored user data
				if (fixture != nullptr)
				{
					auto it = s_PhysicsBodyDataMap.find(fixture);
					SP_CORE_ASSERT(it != s_PhysicsBodyDataMap.end(), "Physics body was not found in Physics Body Data Map!");
					s_PhysicsBodyDataMap.erase(it->first);

					entityRuntimePhysicsBody->DestroyFixture(fixture);
				}
			}

			s_PhysicsScene->DestroyBody(entityRuntimePhysicsBody);
		}
	}

}

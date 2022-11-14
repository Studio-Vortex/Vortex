#include "sppch.h"
#include "Physics3D.h"

#include <q3.h>

namespace Sparky {

	namespace Utils {

		static q3BodyType RigidBodyTypeToQu3eBody(RigidBodyComponent::BodyType bodyType)
		{
			switch (bodyType)
			{
				case Sparky::RigidBodyComponent::BodyType::Static:    return eStaticBody;
				case Sparky::RigidBodyComponent::BodyType::Dynamic:   return eDynamicBody;
				case Sparky::RigidBodyComponent::BodyType::Kinematic: return eKinematicBody;
			}

			SP_CORE_ASSERT(false, "Unknown body type!");
			return eStaticBody;
		}

	}
	
	void Physics3D::OnPhysicsSimulate(entt::registry& sceneRegistry, Scene* contextScene)
	{
		s_PhysicsScene = new q3Scene(1.0f / 120.0f, { s_PhysicsSceneGravity.x, s_PhysicsSceneGravity.y, s_PhysicsSceneGravity.z }, s_PhysicsSceneIterations);

		auto view = sceneRegistry.view<RigidBodyComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };
			auto& transform = entity.GetTransform();
			auto& rb = entity.GetComponent<RigidBodyComponent>();

			CreatePhysicsBody(entity, transform, rb);
		}
	}

	void Physics3D::OnPhysicsUpdate(TimeStep delta, entt::registry& sceneRegistry, Scene* contextScene)
	{
		s_PhysicsScene->SetGravity({ s_PhysicsSceneGravity.x, s_PhysicsSceneGravity.y, s_PhysicsSceneGravity.z });
		s_PhysicsScene->SetIterations(s_PhysicsSceneIterations);

		// Physics
		{
			// Copies transform from Sparky to Qu3e
			auto view = sceneRegistry.view<RigidBodyComponent>();
			for (auto e : view)
			{
				Entity entity = { e, contextScene };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

				// If a rigidbody component is added during runtime we can create the physics body here
				if (rigidbody.RuntimeBody == nullptr)
					CreatePhysicsBody(entity, transform, rigidbody);

				q3Body* body = (q3Body*)rigidbody.RuntimeBody;
				Math::vec3 translation = transform.Translation;
				Math::vec3 rotation = transform.Rotation;

				const auto& bodyPosition = body->GetTransform().position;
				const auto& bodyOrientation = body->GetTransform().rotation;

				bool awake = bodyPosition.x != transform.Translation.x || bodyPosition.y != transform.Translation.y || bodyPosition.z != transform.Translation.z;

				//body->SetTransform({ transform.Translation.x, transform.Translation.y, transform.Translation.z }, { 1.0f, 0.0f, 0.0f }, rotation.x);
				//body->SetTransform({ transform.Translation.x, transform.Translation.y, transform.Translation.z }, { 0.0f, 1.0f, 0.0f }, rotation.y);
				//body->SetTransform({ transform.Translation.x, transform.Translation.y, transform.Translation.z }, { 0.0f, 0.0f, 1.0f }, rotation.z);
				body->SetTransform({ transform.Translation.x, transform.Translation.y,  transform.Translation.z });

				if (awake)
					body->SetToAwake();
			}

			s_PhysicsScene->Step();

			// Get transform from Qu3e
			for (auto e : view)
			{
				Entity entity{ e, contextScene };
				auto& transform = entity.GetComponent<TransformComponent>();
				auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

				if (rigidbody.RuntimeBody == nullptr)
					CreatePhysicsBody(entity, transform, rigidbody);

				q3Body* body = (q3Body*)rigidbody.RuntimeBody;
				const auto& position = body->GetTransform().position;
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Translation.z = position.z;

				//transform.Rotation.z = body->GetTransform().rotation;
			}
		}
	}

	void Physics3D::OnPhysicsStop()
	{
		s_PhysicsScene->Shutdown();
	}

	void Physics3D::CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody)
	{
		q3BodyDef bodyDef;
		bodyDef.bodyType = Utils::RigidBodyTypeToQu3eBody(rigidbody.Type);
		bodyDef.position.Set(transform.Translation.x, transform.Translation.y, transform.Translation.z);
		bodyDef.lockAxisX = rigidbody.ConstrainXAxis;
		bodyDef.lockAxisY = rigidbody.ConstrainYAxis;
		bodyDef.lockAxisZ = rigidbody.ConstrainZAxis;

		q3Body* body = s_PhysicsScene->CreateBody(bodyDef);

		rigidbody.RuntimeBody = body;

		if (entity.HasComponent<BoxColliderComponent>())
		{
			auto& boxCollider = entity.GetComponent<BoxColliderComponent>();

			q3BoxDef boxDef;
			q3Transform t;
			q3Identity(t);
			t.position.Set(transform.Translation.x, transform.Translation.y, transform.Translation.z);
			Math::mat3 tr = Math::mat3((transform.GetTransform()));
			Math::vec3 x = Math::vec3(tr[0][0]);
			Math::vec3 y = Math::vec3(tr[1][0]);
			Math::vec3 z = Math::vec3(tr[2][0]);
			t.rotation = q3Rotate({ x.x, y.z, z.x }, { x.x, y.z, z.z }, { x.x, y.y, z.z });
			boxDef.Set(t, { boxCollider.Size.x * transform.Scale.x, boxCollider.Size.y * transform.Scale.y, boxCollider.Size.z * transform.Scale.z });
			boxDef.SetDensity(boxCollider.Density);
			boxDef.SetFriction(boxCollider.Friction);
			boxDef.SetRestitution(boxCollider.Restitution);

			body->AddBox(boxDef);
		}
	}

	void Physics3D::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body if it exists

		q3Body* entityRuntimePhysicsBody = (q3Body*)entity.GetComponent<RigidBody2DComponent>().RuntimeBody;

		if (entityRuntimePhysicsBody != nullptr)
			s_PhysicsScene->RemoveBody(entityRuntimePhysicsBody);
	}

}

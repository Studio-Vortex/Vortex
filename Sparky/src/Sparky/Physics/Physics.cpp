#include "sppch.h"
#include "Physics.h"

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

		static Math::vec3 QVec3ToVec3(const q3Vec3& v)
		{
			return Math::vec3(v.x, v.y, v.z);
		}
		
		static q3Vec3 Vec3ToQVec3(const Math::vec3& v)
		{
			return q3Vec3(v.x, v.y, v.z);
		}

		static Math::vec3 CalculateEulersFromQMat3(const q3Mat3& m)
		{
			float sy = Math::Sqrt(m.Column0().x * m.Column0().x + m.Column1().x * m.Column1().x);

			bool singular = sy < 1e-6;

			float x, y, z;

			if (!singular)
			{
				x = Math::Atan2(m.Column2().y, m.Column2().z);
				y = Math::Atan2(-m.Column2().x, sy);
				z = Math::Atan2(m.Column1().x, m.Column0().x);
			}
			else
			{
				x = Math::Atan2(-m.Column1().z, m.Column1().y);
				y = Math::Atan2(-m.Column2().x, sy);
				z = 0.0f;
			}

			return Math::vec3(x, y, z);
		}

	}
	
	void Physics::OnSimulationStart(Scene* contextScene)
	{
		s_PhysicsScene = new q3Scene(1.0f / 60.0f, { s_PhysicsSceneGravity.x, s_PhysicsSceneGravity.y, s_PhysicsSceneGravity.z }, s_PhysicsSceneIterations);
		s_PhysicsScene->SetAllowSleep(true);

		auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, contextScene };
			auto& transform = entity.GetTransform();
			auto& rb = entity.GetComponent<RigidBodyComponent>();

			CreatePhysicsBody(entity, transform, rb);
		}
	}

	void Physics::OnSimulationUpdate(TimeStep delta, Scene* contextScene)
	{
		s_PhysicsScene->SetGravity({ s_PhysicsSceneGravity.x, s_PhysicsSceneGravity.y, s_PhysicsSceneGravity.z });
		s_PhysicsScene->SetIterations(s_PhysicsSceneIterations);

		// Physics
		{
			// Copies transform from Sparky to Qu3e
			auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

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

				const auto& bodyTransform = body->GetTransform();
				const auto& bodyPosition = bodyTransform.position;
				const auto& bodyRotation = bodyTransform.rotation;

				Math::mat3 r = Math::mat3(Utils::QVec3ToVec3(bodyRotation.Column0()), Utils::QVec3ToVec3(bodyRotation.Column1()), Utils::QVec3ToVec3(bodyRotation.Column2()));
				
				bool awake = bodyPosition.x != translation.x || bodyPosition.y != translation.y || bodyPosition.z != translation.z;

				body->SetTransform(Utils::Vec3ToQVec3(translation), { 1.0f, 0.0f, 0.0f }, rotation.x);
				body->SetTransform(Utils::Vec3ToQVec3(translation), { 0.0f, 1.0f, 0.0f }, rotation.y);
				body->SetTransform(Utils::Vec3ToQVec3(translation), { 0.0f, 0.0f, 1.0f }, rotation.z);
				if (rigidbody.Velocity != Math::vec3(0.0f) && rigidbody.Type != RigidBodyComponent::BodyType::Static)
					body->SetLinearVelocity(Utils::Vec3ToQVec3(rigidbody.Velocity));
				body->SetLinearDamping(rigidbody.Drag);

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

				q3Body* body = (q3Body*)rigidbody.RuntimeBody;
				const auto& bodyTransform = body->GetTransform();
				transform.Translation = Utils::QVec3ToVec3(bodyTransform.position);
				transform.Rotation = Utils::CalculateEulersFromQMat3(bodyTransform.rotation);
			}
		}
	}

	void Physics::OnSimulationStop()
	{
		s_PhysicsScene->Shutdown();
		delete s_PhysicsScene;
		s_PhysicsScene = nullptr;
	}

	void Physics::CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody)
	{
		q3BodyDef bodyDef;
		bodyDef.bodyType = Utils::RigidBodyTypeToQu3eBody(rigidbody.Type);
		q3Identity(bodyDef.position);
		//bodyDef.position.Set(transform.Translation.x, transform.Translation.y, transform.Translation.z);
		bodyDef.lockAxisX = rigidbody.ConstrainXAxis;
		bodyDef.lockAxisY = rigidbody.ConstrainYAxis;
		bodyDef.lockAxisZ = rigidbody.ConstrainZAxis;
		bodyDef.linearVelocity = Utils::Vec3ToQVec3(rigidbody.Velocity);
		bodyDef.linearDamping = rigidbody.Drag;

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
			Math::vec3 x = Math::vec3(tr[0]);
			Math::vec3 y = Math::vec3(tr[1]);
			Math::vec3 z = Math::vec3(tr[2]);
			t.rotation = q3Rotate({ x.x, y.z, z.x }, { x.x, y.z, z.z }, { x.x, y.y, z.z });
			boxDef.Set(t, { boxCollider.Size.x * transform.Scale.x, boxCollider.Size.y * transform.Scale.y, boxCollider.Size.z * transform.Scale.z });
			boxDef.SetDensity(boxCollider.Density);
			boxDef.SetFriction(boxCollider.Friction);
			boxDef.SetRestitution(boxCollider.Restitution);
			boxDef.SetSensor(boxCollider.IsTrigger);

			body->AddBox(boxDef);
		}
	}

	void Physics::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body if it exists

		q3Body* entityRuntimePhysicsBody = (q3Body*)entity.GetComponent<RigidBodyComponent>().RuntimeBody;

		if (entityRuntimePhysicsBody != nullptr)
			s_PhysicsScene->RemoveBody(entityRuntimePhysicsBody);
	}

}

#include "sppch.h"
#include "PhysicsActor.h"

#include "Sparky/Physics/Physics.h"
#include "Sparky/Physics/PhysXTypes.h"
#include "Sparky/Physics/PhysXAPIHelpers.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Sparky {

	PhysicsActor::PhysicsActor(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody)
		: m_Entity(entity)
	{
		physx::PxRigidActor* actor = nullptr;

		Math::mat4 entityTransform = transform.GetTransform();

		if (rigidbody.Type == RigidBodyType::Static)
		{
			actor = Physics::GetPhysicsFactory()->createRigidStatic(ToPhysXTransform(entityTransform));
		}
		else if (rigidbody.Type == RigidBodyType::Dynamic)
		{
			physx::PxRigidDynamic* dynamicActor = Physics::GetPhysicsFactory()->createRigidDynamic(ToPhysXTransform(entityTransform));
			Physics::UpdateDynamicActorFlags(rigidbody, dynamicActor);
			actor = dynamicActor;
		}

		SP_CORE_ASSERT(actor, "Failed to create Physics Actor!");
		rigidbody.RuntimeActor = actor;

		actor->userData = this;

		if (m_Entity.HasComponent<CharacterControllerComponent>())
		{
			const TransformComponent& transform = m_Entity.GetTransform();
			CharacterControllerComponent& characterController = m_Entity.GetComponent<CharacterControllerComponent>();

			if (m_Entity.HasComponent<CapsuleColliderComponent>())
			{
				const auto& capsuleCollider = m_Entity.GetComponent<CapsuleColliderComponent>();

				physx::PxMaterial* material = nullptr;

				if (m_Entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = m_Entity.GetComponent<PhysicsMaterialComponent>();
					material = Physics::GetPhysicsFactory()->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
				}
				else
				{
					// Create a default material
					material = Physics::GetPhysicsFactory()->createMaterial(1.0f, 1.0f, 1.0f);
				}

				float radiusScale = Math::Max(transform.Scale.x, transform.Scale.y);

				physx::PxCapsuleControllerDesc desc;
				desc.position = ToPhysxExtendedVector(transform.Translation + capsuleCollider.Offset);
				desc.height = capsuleCollider.Height * transform.Scale.y;
				desc.radius = capsuleCollider.Radius * radiusScale;
				desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING; // TODO: get from component
				desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;
				desc.slopeLimit = Math::Max(0.0f, Math::Cos(Math::Deg2Rad(characterController.SlopeLimitDegrees)));
				desc.stepOffset = characterController.StepOffset;
				desc.contactOffset = 0.01f; // TODO: get from component
				desc.material = material;
				desc.upDirection = { 0.0f, 1.0f, 0.0f };

				characterController.RuntimeController = Physics::GetControllerManager()->createController(desc);
			}
			else if (m_Entity.HasComponent<BoxColliderComponent>())
			{
				const auto& boxCollider = m_Entity.GetComponent<BoxColliderComponent>();

				physx::PxMaterial* material = nullptr;

				if (m_Entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = m_Entity.GetComponent<PhysicsMaterialComponent>();
					material = Physics::GetPhysicsFactory()->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
				}
				else
				{
					// Create a default material
					material = Physics::GetPhysicsFactory()->createMaterial(1.0f, 1.0f, 1.0f);
				}

				physx::PxBoxControllerDesc desc;
				desc.position = ToPhysxExtendedVector(transform.Translation + boxCollider.Offset);
				desc.halfHeight = (boxCollider.HalfSize.y * transform.Scale.y);
				desc.halfSideExtent = (boxCollider.HalfSize.x * transform.Scale.x);
				desc.halfForwardExtent = (boxCollider.HalfSize.z * transform.Scale.z);
				desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING; // TODO: get from component
				desc.slopeLimit = Math::Max(0.0f, Math::Cos(Math::Deg2Rad(characterController.SlopeLimitDegrees)));
				desc.stepOffset = characterController.StepOffset;
				desc.contactOffset = 0.01f; // TODO: get from component
				desc.material = material;
				desc.upDirection = { 0.0f, 1.0f, 0.0f };

				characterController.RuntimeController = Physics::GetControllerManager()->createController(desc);
			}
		}
		else
		{
			if (m_Entity.HasComponent<BoxColliderComponent>())
			{
				const BoxColliderComponent& boxCollider = m_Entity.GetComponent<BoxColliderComponent>();
				Math::vec3 scale = transform.Scale;

				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
				physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(boxCollider.HalfSize.x * scale.x, boxCollider.HalfSize.y * scale.y, boxCollider.HalfSize.z * scale.z);
				physx::PxMaterial* material = nullptr;

				if (m_Entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = m_Entity.GetComponent<PhysicsMaterialComponent>();
					material = Physics::GetPhysicsFactory()->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
				}
				else
				{
					// Create a default material
					material = Physics::GetPhysicsFactory()->createMaterial(1.0f, 1.0f, 1.0f);
				}

				physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, boxGeometry, *material);
				shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !boxCollider.IsTrigger);
				shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, boxCollider.IsTrigger);
				shape->setLocalPose(ToPhysXTransform(Math::Translate(boxCollider.Offset)));
			}

			if (m_Entity.HasComponent<SphereColliderComponent>())
			{
				auto& sphereCollider = m_Entity.GetComponent<SphereColliderComponent>();

				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
				physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(sphereCollider.Radius);
				physx::PxMaterial* material = nullptr;

				if (m_Entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = m_Entity.GetComponent<PhysicsMaterialComponent>();
					material = Physics::GetPhysicsFactory()->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
				}
				else
				{
					// Create a default material
					material = Physics::GetPhysicsFactory()->createMaterial(1.0f, 1.0f, 1.0f);
				}

				physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, sphereGeometry, *material);
				shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !sphereCollider.IsTrigger);
				shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, sphereCollider.IsTrigger);
			}

			if (m_Entity.HasComponent<CapsuleColliderComponent>())
			{
				const auto& capsuleCollider = m_Entity.GetComponent<CapsuleColliderComponent>();

				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
				physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(capsuleCollider.Radius, capsuleCollider.Height / 2.0F);
				physx::PxMaterial* material = nullptr;

				if (m_Entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = m_Entity.GetComponent<PhysicsMaterialComponent>();
					material = Physics::GetPhysicsFactory()->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
				}
				else
				{
					// Create a default material
					material = Physics::GetPhysicsFactory()->createMaterial(1.0f, 1.0f, 1.0f);
				}

				physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, capsuleGeometry, *material);
				shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !capsuleCollider.IsTrigger);
				shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, capsuleCollider.IsTrigger);

				// Make sure that the capsule is facing up (+Y)
				shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
			}
		}

		// Set Filters
		if (rigidbody.Type == RigidBodyType::Static)
			Physics::SetCollisionFilters(actor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);
		else if (rigidbody.Type == RigidBodyType::Dynamic)
			Physics::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);

		Physics::GetPhysicsScene()->addActor(*actor);
	}

}

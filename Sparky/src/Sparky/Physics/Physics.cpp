#include "sppch.h"
#include "Physics.h"

#include <PhysX/PxPhysicsAPI.h>

namespace Sparky {

	struct PhysicsEngineInternalData
	{
		physx::PxDefaultAllocator DefaultAllocator;
		physx::PxDefaultErrorCallback ErrorCallback;
		physx::PxFoundation* Foundation = nullptr;
		physx::PxPhysics* PhysicsFactory = nullptr;
		physx::PxDefaultCpuDispatcher* Dispatcher = nullptr;
		physx::PxScene* PhysicsScene = nullptr;
		physx::PxTolerancesScale ToleranceScale;
	};

	static PhysicsEngineInternalData s_Data;

	namespace Utils {

		static std::tuple<Math::vec3, Math::quaternion, Math::vec3> GetTransformDecomposition(const Math::mat4& transform)
		{
			Math::vec3 scale, translation, skew;
			Math::vec4 perspective;
			Math::quaternion orientation;
			Math::Decompose(transform, scale, orientation, translation, skew, perspective);

			return { translation, orientation, scale };
		}

		static physx::PxTransform SparkyTransformToPxTransform(const Math::mat4& transform)
		{
			auto [translation, rotationQuat, scale] = GetTransformDecomposition(transform);
			Math::vec3 rotation = Math::EulerAngles(rotationQuat);

			physx::PxTransform physxTransform(physx::PxVec3(translation.x, translation.y, translation.z));
			physxTransform.rotate(physx::PxVec3(rotation.x, rotation.y, rotation.z));
			return physxTransform;
		}
		
		static void SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask)
		{
			physx::PxFilterData filterData;
			filterData.word0 = filterGroup; // word0 = own ID
			filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a

			// contact callback;
			const physx::PxU32 numShapes = actor->getNbShapes();
			physx::PxShape** shapes = (physx::PxShape**)s_Data.DefaultAllocator.allocate(sizeof(physx::PxShape*) * numShapes, "", "", 0);
			actor->getShapes(shapes, numShapes);

			for (physx::PxU32 i = 0; i < numShapes; i++)
			{
				physx::PxShape* shape = shapes[i];
				shape->setSimulationFilterData(filterData);
			}

			s_Data.DefaultAllocator.deallocate(shapes);
		}
	}

	static void InitPhysicsScene(const Math::vec3& gravity)
	{
		// Create the scene with the description
		s_Data.Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Data.DefaultAllocator, s_Data.ErrorCallback);
		SP_CORE_ASSERT(s_Data.Foundation, "Failed to create Physics Scene Foundation!");

		s_Data.ToleranceScale.length = 100.0f; // Typical length of an object
		s_Data.ToleranceScale.speed = 981.0f; // Typical speed of an object, gravity * speed is a reasonable choice
		s_Data.PhysicsFactory = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Data.Foundation, s_Data.ToleranceScale, true, nullptr);

		physx::PxSceneDesc sceneDescription = physx::PxSceneDesc(s_Data.ToleranceScale);
		sceneDescription.gravity = physx::PxVec3(gravity.x, gravity.y, gravity.z);

		s_Data.Dispatcher = physx::PxDefaultCpuDispatcherCreate(2);
		sceneDescription.cpuDispatcher = s_Data.Dispatcher;
		sceneDescription.filterShader = physx::PxDefaultSimulationFilterShader;
		s_Data.PhysicsScene = s_Data.PhysicsFactory->createScene(sceneDescription);
	}

	void Physics::OnSimulationStart(Scene* contextScene)
	{
		InitPhysicsScene(s_PhysicsSceneGravity);

		auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

		for (auto& e : view)
		{
			Entity entity{ e, contextScene };
			CreatePhysicsBody(entity, entity.GetTransform(), entity.GetComponent<RigidBodyComponent>());
		}
	}

	void Physics::OnSimulationUpdate(TimeStep delta, Scene* contextScene)
	{
		constexpr float stepSize = 0.016666660f;
		s_Data.PhysicsScene->simulate(stepSize);
		s_Data.PhysicsScene->fetchResults(true);

		auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

		for (auto& e : view)
		{
			Entity entity{ e, contextScene };
			auto& transform = entity.GetTransform();
			auto trx = transform.GetTransform();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (!rigidbody.RuntimeActor)
				CreatePhysicsBody(entity, entity.GetTransform(), rigidbody);

			physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
			const auto& position = actor->getGlobalPose().p;
			const physx::PxQuat& physicsBodyRotation = actor->getGlobalPose().q;

			auto [translation, rotationQuat, scale] = Utils::GetTransformDecomposition(trx);
			Math::vec3 rotation = Math::EulerAngles(rotationQuat);

			if (rigidbody.Type == RigidBodyComponent::BodyType::Dynamic)
			{
				// If the rigidbody is dynamic, the position of the entity is determined by the rigidbody
				// TODO: Get rotation from RigidActor
				float xAngle, yAngle, zAngle;
				physx::PxVec3 axis;
				physicsBodyRotation.toRadiansAndUnitAxis(xAngle, axis);
				physicsBodyRotation.toRadiansAndUnitAxis(yAngle, axis);
				physicsBodyRotation.toRadiansAndUnitAxis(zAngle, axis);

				trx = Math::Translate({ position.x, position.y, position.z }) *
					Math::ToMat4(Math::quaternion({ xAngle, yAngle, zAngle })) *
					Math::Scale(scale);

				transform.Translation = { position.x, position.y, position.z };
				transform.Rotation = { xAngle, yAngle, zAngle };
			}
			else if (rigidbody.Type == RigidBodyComponent::BodyType::Static)
			{
				// If the rigidbody is static, make sure the actor is at the entitys position
				actor->setGlobalPose(Utils::SparkyTransformToPxTransform(trx));
			}
		}
	}

	void Physics::OnSimulationStop()
	{
		s_Data.PhysicsScene->release();
		s_Data.PhysicsFactory->release();
		s_Data.Foundation->release();
	}

	void Physics::CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody)
	{
		physx::PxRigidActor* actor = nullptr;

		Math::mat4 trx = transform.GetTransform();

		if (rigidbody.Type == RigidBodyComponent::BodyType::Static)
		{
			actor = s_Data.PhysicsFactory->createRigidStatic(Utils::SparkyTransformToPxTransform(trx));
		}
		else if (rigidbody.Type == RigidBodyComponent::BodyType::Dynamic)
		{
			physx::PxRigidDynamic* dynamicActor = s_Data.PhysicsFactory->createRigidDynamic(Utils::SparkyTransformToPxTransform(trx));
			physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidbody.Mass);

			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, rigidbody.LockPositionX);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, rigidbody.LockPositionY);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, rigidbody.LockPositionZ);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rigidbody.LockRotationX);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rigidbody.LockRotationY);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rigidbody.LockRotationZ);

			actor = dynamicActor;
		}

		SP_CORE_ASSERT(actor, "Failed to create Physics Actor!");
		rigidbody.RuntimeActor = actor;
		s_Data.PhysicsScene->addActor(*actor);

		if (entity.HasComponent<BoxColliderComponent>())
		{
			const BoxColliderComponent& boxCollider = entity.GetComponent<BoxColliderComponent>();

			physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
			physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(boxCollider.HalfSize.x, boxCollider.HalfSize.y, boxCollider.HalfSize.z);
			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = s_Data.PhysicsFactory->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
			}
			else
			{
				// Create a default material
				material = s_Data.PhysicsFactory->createMaterial(1.0f, 1.0f, 1.0f);
			}

			physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, boxGeometry, *material);
			shape->setLocalPose(Utils::SparkyTransformToPxTransform(Math::Translate(boxCollider.Offset)));
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			auto& sphereCollider = entity.GetComponent<SphereColliderComponent>();

			physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
			physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(sphereCollider.Radius);
			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = s_Data.PhysicsFactory->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
			}
			else
			{
				// Create a default material
				material = s_Data.PhysicsFactory->createMaterial(1.0f, 1.0f, 1.0f);
			}

			physx::PxRigidActorExt::createExclusiveShape(*actor, sphereGeometry, *material);

			physx::PxRigidDynamic* rigidBodyActor = actor->is<physx::PxRigidDynamic>();

			if (rigidBodyActor)
			{
				rigidBodyActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, true);
				rigidBodyActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, true);
				rigidBodyActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, true);
			}
		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			const auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();

			physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
			physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(capsuleCollider.Radius, capsuleCollider.Height / 2.0F);
			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = s_Data.PhysicsFactory->createMaterial(physicsMaterial.StaticFriction, physicsMaterial.DynamicFriction, physicsMaterial.Bounciness);
			}
			else
			{
				// Create a default material
				material = s_Data.PhysicsFactory->createMaterial(1.0f, 1.0f, 1.0f);
			}

			physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, capsuleGeometry, *material);

			// Make sure that the capsule is facing up (+Y)
			shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
		}

		// Set Filters
		if (rigidbody.Type == RigidBodyComponent::BodyType::Static)
			Utils::SetCollisionFilters(actor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);
		else if (rigidbody.Type == RigidBodyComponent::BodyType::Dynamic)
			Utils::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
	}

	void Physics::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body if it exists

		
	}

}

#include "sppch.h"
#include "Physics.h"

#include "Sparky/Physics/PhysXAPIHelpers.h"

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

		s_Data.Dispatcher = physx::PxDefaultCpuDispatcherCreate(1);
		sceneDescription.cpuDispatcher = s_Data.Dispatcher;
		sceneDescription.filterShader = physx::PxDefaultSimulationFilterShader;
		s_Data.PhysicsScene = s_Data.PhysicsFactory->createScene(sceneDescription);
	}

	physx::PxScene* Physics::GetPhysicsScene()
	{
		return s_Data.PhysicsScene;
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
			auto [t, r, scale] = GetTransformDecomposition(trx);

			if (rigidbody.Type == RigidBodyComponent::BodyType::Dynamic)
			{
				entity.SetTransform(FromPhysXTransform(actor->getGlobalPose()) * Math::Scale(scale));
			}
			else if (rigidbody.Type == RigidBodyComponent::BodyType::Static)
			{
				// If the rigidbody is static, make sure the actor is at the entitys position
				actor->setGlobalPose(ToPhysXTransform(trx));
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
			actor = s_Data.PhysicsFactory->createRigidStatic(ToPhysXTransform(trx));
		}
		else if (rigidbody.Type == RigidBodyComponent::BodyType::Dynamic)
		{
			physx::PxRigidDynamic* dynamicActor = s_Data.PhysicsFactory->createRigidDynamic(ToPhysXTransform(trx));

			dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, rigidbody.IsKinematic);

			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, rigidbody.LockPositionX);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, rigidbody.LockPositionY);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, rigidbody.LockPositionZ);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rigidbody.LockRotationX);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rigidbody.LockRotationY);
			dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rigidbody.LockRotationZ);

			physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidbody.Mass);
			actor = dynamicActor;
		}

		SP_CORE_ASSERT(actor, "Failed to create Physics Actor!");
		rigidbody.RuntimeActor = actor;

		if (entity.HasComponent<BoxColliderComponent>())
		{
			const BoxColliderComponent& boxCollider = entity.GetComponent<BoxColliderComponent>();
			Math::vec3 scale = transform.Scale;

			physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
			physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(boxCollider.HalfSize.x * scale.x, boxCollider.HalfSize.y * scale.y, boxCollider.HalfSize.z * scale.z);
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
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !boxCollider.IsTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, boxCollider.IsTrigger);
			shape->setLocalPose(ToPhysXTransform(Math::Translate(boxCollider.Offset)));
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

			physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, sphereGeometry, *material);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !sphereCollider.IsTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, sphereCollider.IsTrigger);
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
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !capsuleCollider.IsTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, capsuleCollider.IsTrigger);

			// Make sure that the capsule is facing up (+Y)
			shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
		}

		// Set Filters
		if (rigidbody.Type == RigidBodyComponent::BodyType::Static)
			Utils::SetCollisionFilters(actor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);
		else if (rigidbody.Type == RigidBodyComponent::BodyType::Dynamic)
			Utils::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);

		s_Data.PhysicsScene->addActor(*actor);
	}

	void Physics::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body if it exists

		
	}

}

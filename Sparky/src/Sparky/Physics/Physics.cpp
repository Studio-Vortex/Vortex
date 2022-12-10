#include "sppch.h"
#include "Physics.h"

#include "Sparky/Utils/PlatformUtils.h"
#include "Sparky/Physics/PhysXTypes.h"
#include "Sparky/Physics/PhysicsActor.h"
#include "Sparky/Physics/PhysXAPIHelpers.h"

namespace Sparky {

	struct PhysicsEngineInternalData
	{
		physx::PxDefaultAllocator DefaultAllocator;
		physx::PxDefaultErrorCallback ErrorCallback;
		physx::PxFoundation* Foundation = nullptr;
		physx::PxPhysics* PhysicsFactory = nullptr;
		physx::PxDefaultCpuDispatcher* Dispatcher = nullptr;
		physx::PxCooking* CookingFactory = nullptr;
		physx::PxControllerManager* ControllerManager = nullptr;
		physx::PxScene* PhysicsScene = nullptr;
		physx::PxTolerancesScale TolerancesScale;
	};

	static PhysicsEngineInternalData* s_Data = nullptr;

	namespace Utils {

		static void ReplaceNonExistantVectorAxis(Math::vec3& vector, const physx::PxVec3& replacementVector)
		{
			if (vector.x == 0.0f)
				vector.x = replacementVector.x;
			if (vector.y == 0.0f)
				vector.y = replacementVector.y;
			if (vector.z == 0.0f)
				vector.z = replacementVector.z;
		}

	}

	static void InitPhysicsScene(const Math::vec3& gravity)
	{
		s_Data = new PhysicsEngineInternalData();

		// Create the scene with the description
		s_Data->Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Data->DefaultAllocator, s_Data->ErrorCallback);
		SP_CORE_ASSERT(s_Data->Foundation, "Failed to create Physics Scene Foundation!");

		s_Data->TolerancesScale = physx::PxTolerancesScale();
		s_Data->TolerancesScale.length = 1.0; // Typical length of an object
		s_Data->TolerancesScale.speed = 100.0f; // Typical speed of an object, gravity * speed is a reasonable choice
		s_Data->PhysicsFactory = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Data->Foundation, s_Data->TolerancesScale, true, nullptr);

		bool extentionsLoaded = PxInitExtensions(*s_Data->PhysicsFactory, nullptr);
		SP_CORE_ASSERT(extentionsLoaded, "Failed to initialize PhysX Extensions");

		s_Data->Dispatcher = physx::PxDefaultCpuDispatcherCreate(1);

		physx::PxCookingParams cookingParameters = physx::PxCookingParams(s_Data->TolerancesScale);
		cookingParameters.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

		s_Data->CookingFactory = PxCreateCooking(PX_PHYSICS_VERSION, *s_Data->Foundation, cookingParameters);
		SP_CORE_ASSERT(s_Data->CookingFactory, "Failed to Initialize PhysX Cooking!");

		physx::PxSceneDesc sceneDescription = physx::PxSceneDesc(s_Data->TolerancesScale);
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDescription.gravity = ToPhysXVector(gravity);
		sceneDescription.broadPhaseType = physx::PxBroadPhaseType::eABP; // May potenially want different options
		sceneDescription.frictionType = physx::PxFrictionType::ePATCH; // Same here

		sceneDescription.cpuDispatcher = s_Data->Dispatcher;
		sceneDescription.filterShader = physx::PxDefaultSimulationFilterShader;

		s_Data->PhysicsScene = s_Data->PhysicsFactory->createScene(sceneDescription);
		s_Data->ControllerManager = PxCreateControllerManager(*s_Data->PhysicsScene);
	}

	physx::PxScene* Physics::GetPhysicsScene()
	{
		return s_Data->PhysicsScene;
	}

	physx::PxPhysics* Physics::GetPhysicsFactory()
	{
		return s_Data->PhysicsFactory;
	}

	physx::PxControllerManager* Physics::GetControllerManager()
	{
		return s_Data->ControllerManager;
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
		s_Data->PhysicsScene->simulate(s_FixedTimeStep);
		s_Data->PhysicsScene->fetchResults(true);

		auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

		for (auto& e : view)
		{
			Entity entity{ e, contextScene };
			auto& transform = entity.GetTransform();
			const auto trx = transform.GetTransform();
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (!rigidbody.RuntimeActor)
			{
				CreatePhysicsBody(entity, entity.GetTransform(), rigidbody);
			}

			physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
			auto [t, r, scale] = GetTransformDecomposition(trx);

			if (rigidbody.Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* dynamicActor = static_cast<physx::PxRigidDynamic*>(actor);

				entity.SetTransform(FromPhysXTransform(dynamicActor->getGlobalPose()) * Math::Scale(scale));
				if (actor->is<physx::PxRigidDynamic>())
				{
					UpdateDynamicActorFlags(rigidbody, dynamicActor);
				}
			}
			else if (rigidbody.Type == RigidBodyType::Static)
			{
				// If the rigidbody is static, make sure the actor is at the entitys position
				actor->setGlobalPose(ToPhysXTransform(trx));
			}

			// Synchronize controller transform
			if (entity.HasComponent<CharacterControllerComponent>())
			{
				auto& characterController = entity.GetComponent<CharacterControllerComponent>();
				physx::PxController* controller = static_cast<physx::PxController*>(characterController.RuntimeController);

				Math::vec3 position = FromPhysXExtendedVector(controller->getPosition());

				if (entity.HasComponent<CapsuleColliderComponent>())
				{
					const auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();
					position -= capsuleCollider.Offset;
				}
				else if (entity.HasComponent<BoxColliderComponent>())
				{
					const auto& boxCollider = entity.GetComponent<BoxColliderComponent>();
					position -= boxCollider.Offset;
				}

				controller->setStepOffset(characterController.StepOffset);
				controller->setSlopeLimit(Math::Deg2Rad(characterController.SlopeLimitDegrees));

				transform.Translation = position;
			}
		}
	}

	void Physics::OnSimulationStop()
	{
		if (s_Data)
		{
			s_Data->ControllerManager->release();
			s_Data->PhysicsScene->release();
			s_Data->CookingFactory->release();
			s_Data->Dispatcher->release();
			PxCloseExtensions();
			s_Data->PhysicsFactory->release();
			s_Data->Foundation->release();

			delete s_Data;
		}
	}

	void Physics::CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody)
	{
		PhysicsActor(entity, transform, rigidbody);
	}

	void Physics::SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask)
	{
		physx::PxFilterData filterData;
		filterData.word0 = filterGroup; // word0 = own ID
		filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a

		// contact callback;
		const physx::PxU32 numShapes = actor->getNbShapes();

		physx::PxShape** shapes = (physx::PxShape**)s_Data->DefaultAllocator.allocate(sizeof(physx::PxShape*) * numShapes, "", "", 0);
		actor->getShapes(shapes, numShapes);

		for (physx::PxU32 i = 0; i < numShapes; i++)
		{
			physx::PxShape* shape = shapes[i];
			shape->setSimulationFilterData(filterData);
		}

		s_Data->DefaultAllocator.deallocate(shapes);
	}

	void Physics::UpdateDynamicActorFlags(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor)
	{
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, rigidbody.IsKinematic);
		dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, rigidbody.DisableGravity);

		dynamicActor->setSolverIterationCounts(s_PhysicsSolverIterations,  s_PhysicsSolverVelocityIterations);

		dynamicActor->setMass(rigidbody.Mass);

		if (rigidbody.LinearVelocity != Math::vec3(0.0f))
		{
			Math::vec3 linearVelocity = rigidbody.LinearVelocity;
			physx::PxVec3 actorVelocity = dynamicActor->getLinearVelocity();
			// If any component of the vector is 0 just use the actors velocity
			Utils::ReplaceNonExistantVectorAxis(linearVelocity, actorVelocity);

			dynamicActor->setLinearVelocity(ToPhysXVector(linearVelocity));
		}

		dynamicActor->setLinearDamping(rigidbody.LinearDrag);

		if (rigidbody.AngularVelocity != Math::vec3(0.0f))
		{
			Math::vec3 angularVelocity = rigidbody.AngularVelocity;
			physx::PxVec3 actorVelocity = dynamicActor->getAngularVelocity();
			// If any component of the vector is 0 just use the actors velocity
			Utils::ReplaceNonExistantVectorAxis(angularVelocity, actorVelocity);

			dynamicActor->setAngularVelocity(ToPhysXVector(angularVelocity));
		}

		dynamicActor->setAngularDamping(rigidbody.AngularDrag);

		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X, rigidbody.LockPositionX);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y, rigidbody.LockPositionY);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z, rigidbody.LockPositionZ);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rigidbody.LockRotationX);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rigidbody.LockRotationY);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rigidbody.LockRotationZ);

		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, true);
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, true);

		physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidbody.Mass);
	}

	void Physics::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body if it exists

		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
		physx::PxActor* actor = static_cast<physx::PxActor*>(rigidbody.RuntimeActor);

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			physx::PxController* controller = static_cast<physx::PxController*>(entity.GetComponent<CharacterControllerComponent>().RuntimeController);
			controller->release();
		}

		if (actor != nullptr)
		{
			// Remove body from internal map


			s_Data->PhysicsScene->removeActor(*actor);
		}
	}

}

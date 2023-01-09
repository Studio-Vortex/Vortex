#include "vxpch.h"
#include "Physics.h"

#include "Vortex/Utils/PlatformUtils.h"
#include "Vortex/Physics/PhysXTypes.h"
#include "Vortex/Physics/PhysXAPIHelpers.h"
#include "Vortex/Scripting/ScriptEngine.h"

namespace Vortex {

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

		Scene* ContextScene = nullptr;
	};

	static PhysicsEngineInternalData* s_Data = nullptr;

	namespace Utils {

		static void ReplaceInconsistentVectorAxis(Math::vec3& vector, const physx::PxVec3& replacementVector)
		{
			uint32_t size = vector.length();
			for (uint32_t i = 0; i < size; i++)
			{
				if (vector[i] == 0.0f)
					vector[i] = replacementVector[i];
			}
		}

	}

	class PhysicsContactListener : public physx::PxSimulationEventCallback
	{
	public:
		~PhysicsContactListener() override = default;

		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override
		{
			PX_UNUSED(constraints);
			PX_UNUSED(count);
		}

		void onWake(physx::PxActor** actors, physx::PxU32 count) override
		{
			PX_UNUSED(actors);
			PX_UNUSED(count);
		}

		void onSleep(physx::PxActor** actors, physx::PxU32 count) override
		{
			PX_UNUSED(actors);
			PX_UNUSED(count);
		}

		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override
		{
			bool removedActorA = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_0;
			bool removedActorB = pairHeader.flags & physx::PxContactPairHeaderFlag::eREMOVED_ACTOR_1;

			PhysicsBodyData* entityDataA = (PhysicsBodyData*)pairHeader.actors[0]->userData;
			PhysicsBodyData* entityDataB = (PhysicsBodyData*)pairHeader.actors[1]->userData;

			if (!entityDataA || !entityDataB)
				return;

			Entity entityA = s_Data->ContextScene->TryGetEntityWithUUID(entityDataA->EntityUUID);
			Entity entityB = s_Data->ContextScene->TryGetEntityWithUUID(entityDataB->EntityUUID);

			if (!entityA || !entityB)
				return;

			if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH)
			{
				if (!entityA.HasComponent<ScriptComponent>() || !entityB.HasComponent<ScriptComponent>())
					return;

				Collision collisionA{};
				collisionA.EntityID = entityB.GetUUID();
				ScriptEngine::OnCollisionBeginEntity(entityA, entityB, collisionA);

				Collision collisionB{};
				collisionB.EntityID = entityA.GetUUID();
				ScriptEngine::OnCollisionBeginEntity(entityB, entityA, collisionB);
			}
			else if (pairs->flags == physx::PxContactPairFlag::eACTOR_PAIR_LOST_TOUCH)
			{
				if (!entityA.HasComponent<ScriptComponent>() || !entityB.HasComponent<ScriptComponent>())
					return;

				Collision collisionA{};
				collisionA.EntityID = entityB.GetUUID();
				ScriptEngine::OnCollisionEndEntity(entityA, entityB, collisionA);

				Collision collisionB{};
				collisionB.EntityID = entityA.GetUUID();
				ScriptEngine::OnCollisionEndEntity(entityB, entityA, collisionB);
			}

			VX_CORE_INFO("Physics::OnCollision, A: {}, B: {}", entityA.GetName(), entityB.GetName());
		}

		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
		{
			for (uint32_t i = 0; i < count; i++)
			{
				if (pairs[i].flags & (physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER | physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
					continue;

				PhysicsBodyData* triggerActorPhysicsBodyData = (PhysicsBodyData*)pairs[i].triggerActor->userData;
				PhysicsBodyData* otherActorPhysicsBodyData = (PhysicsBodyData*)pairs[i].otherActor->userData;

				if (!triggerActorPhysicsBodyData || !otherActorPhysicsBodyData)
					continue;

				Entity triggerEntity = s_Data->ContextScene->TryGetEntityWithUUID(triggerActorPhysicsBodyData->EntityUUID);
				Entity otherEntity = s_Data->ContextScene->TryGetEntityWithUUID(otherActorPhysicsBodyData->EntityUUID);

				if (!triggerEntity || !otherEntity)
					continue;

				if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_CCD)
				{
					if (!triggerEntity.HasComponent<ScriptComponent>() || !otherEntity.HasComponent<ScriptComponent>())
						return;

					ScriptEngine::OnTriggerBeginEntity(triggerEntity, otherEntity);
					ScriptEngine::OnTriggerBeginEntity(otherEntity, triggerEntity);
				}
				else if (pairs[i].status == physx::PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					if (!triggerEntity.HasComponent<ScriptComponent>() || !otherEntity.HasComponent<ScriptComponent>())
						return;

					ScriptEngine::OnTriggerEndEntity(triggerEntity, otherEntity);
					ScriptEngine::OnTriggerEndEntity(otherEntity, triggerEntity);
				}

				VX_CORE_INFO("Physics::OnTrigger, trigger: {}, other: {}", triggerEntity.GetName(), otherEntity.GetName());
			}
		}

		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override
		{
			PX_UNUSED(bodyBuffer);
			PX_UNUSED(poseBuffer);
			PX_UNUSED(count);
		}
	};

	void Physics::Init()
	{
		s_Data = new PhysicsEngineInternalData();

		// Create the scene with the description
		s_Data->Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Data->DefaultAllocator, s_Data->ErrorCallback);
		VX_CORE_ASSERT(s_Data->Foundation, "Failed to create Physics Scene Foundation!");

		s_Data->TolerancesScale = physx::PxTolerancesScale();
		s_Data->TolerancesScale.length = 1.0; // Typical length of an object
		s_Data->TolerancesScale.speed = 100.0f; // Typical speed of an object, gravity * speed is a reasonable choice
		s_Data->PhysicsFactory = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Data->Foundation, s_Data->TolerancesScale, true, nullptr);

		bool extentionsLoaded = PxInitExtensions(*s_Data->PhysicsFactory, nullptr);
		VX_CORE_ASSERT(extentionsLoaded, "Failed to initialize PhysX Extensions");

		s_Data->Dispatcher = physx::PxDefaultCpuDispatcherCreate(1);

		physx::PxCookingParams cookingParameters = physx::PxCookingParams(s_Data->TolerancesScale);
		cookingParameters.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

		s_Data->CookingFactory = PxCreateCooking(PX_PHYSICS_VERSION, *s_Data->Foundation, cookingParameters);
		VX_CORE_ASSERT(s_Data->CookingFactory, "Failed to Initialize PhysX Cooking!");
	}

	void Physics::Shutdown()
	{
		s_Data->CookingFactory->release();
		s_Data->Dispatcher->release();
		PxCloseExtensions();
		s_Data->PhysicsFactory->release();
		s_Data->Foundation->release();

		delete s_Data;
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
		physx::PxSceneDesc sceneDescription = physx::PxSceneDesc(s_Data->TolerancesScale);
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDescription.gravity = ToPhysXVector(s_PhysicsSceneGravity);
		sceneDescription.broadPhaseType = physx::PxBroadPhaseType::eABP; // May potenially want different options
		sceneDescription.frictionType = physx::PxFrictionType::ePATCH; // Same here

		sceneDescription.cpuDispatcher = s_Data->Dispatcher;
		sceneDescription.filterShader = physx::PxDefaultSimulationFilterShader;

		s_Data->PhysicsScene = s_Data->PhysicsFactory->createScene(sceneDescription);
		s_Data->ControllerManager = PxCreateControllerManager(*s_Data->PhysicsScene);

		PhysicsContactListener contactListener;
		s_Data->PhysicsScene->setSimulationEventCallback(&contactListener);

		auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, contextScene };
			CreatePhysicsBody(entity, entity.GetTransform(), entity.GetComponent<RigidBodyComponent>());
		}

		s_Data->ContextScene = contextScene;
	}

	void Physics::OnSimulationUpdate(TimeStep delta, Scene* contextScene)
	{
		s_Data->PhysicsScene->simulate(s_FixedTimeStep);
		s_Data->PhysicsScene->fetchResults(true);

		auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

		for (const auto& e : view)
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

	void Physics::OnSimulationStop(Scene* contextScene)
	{
		auto view = contextScene->GetAllEntitiesWith<RigidBodyComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, contextScene };
			DestroyPhysicsBody(entity);
		}

		s_Data->ControllerManager->release();
		s_Data->ControllerManager = nullptr;
		s_Data->PhysicsScene->release();
		s_Data->PhysicsScene = nullptr;

		s_Data->ContextScene = nullptr;
	}

	void Physics::CreatePhysicsBody(Entity entity, const TransformComponent& transform, RigidBodyComponent& rigidbody)
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

		VX_CORE_ASSERT(actor, "Failed to create Physics Actor!");
		rigidbody.RuntimeActor = actor;

		PhysicsBodyData* physicsBodyData = new PhysicsBodyData();
		physicsBodyData->EntityUUID = entity.GetUUID();
		actor->userData = physicsBodyData;

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			const TransformComponent& transform = entity.GetTransform();
			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();

			if (entity.HasComponent<CapsuleColliderComponent>())
			{
				const auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();

				physx::PxMaterial* material = nullptr;

				if (entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
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

				characterController.RuntimeController = s_Data->ControllerManager->createController(desc);
			}
			else if (entity.HasComponent<BoxColliderComponent>())
			{
				const auto& boxCollider = entity.GetComponent<BoxColliderComponent>();

				physx::PxMaterial* material = nullptr;

				if (entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
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

				characterController.RuntimeController = s_Data->ControllerManager->createController(desc);
			}
		}
		else
		{
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

			if (entity.HasComponent<SphereColliderComponent>())
			{
				auto& sphereCollider = entity.GetComponent<SphereColliderComponent>();

				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
				physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(sphereCollider.Radius);
				physx::PxMaterial* material = nullptr;

				if (entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
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

			if (entity.HasComponent<CapsuleColliderComponent>())
			{
				const auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();

				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);
				physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(capsuleCollider.Radius, capsuleCollider.Height / 2.0F);
				physx::PxMaterial* material = nullptr;

				if (entity.HasComponent<PhysicsMaterialComponent>())
				{
					const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
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

		s_Data->PhysicsScene->addActor(*actor);
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
			Utils::ReplaceInconsistentVectorAxis(linearVelocity, actorVelocity);

			dynamicActor->setLinearVelocity(ToPhysXVector(linearVelocity));
		}

		dynamicActor->setLinearDamping(rigidbody.LinearDrag);

		if (rigidbody.AngularVelocity != Math::vec3(0.0f))
		{
			Math::vec3 angularVelocity = rigidbody.AngularVelocity;
			physx::PxVec3 actorVelocity = dynamicActor->getAngularVelocity();
			// If any component of the vector is 0 just use the actors velocity
			Utils::ReplaceInconsistentVectorAxis(angularVelocity, actorVelocity);

			dynamicActor->setAngularVelocity(ToPhysXVector(angularVelocity));
		}

		dynamicActor->setAngularDamping(rigidbody.AngularDrag);

		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_X,  rigidbody.LockPositionX);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Y,  rigidbody.LockPositionY);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_LINEAR_Z,  rigidbody.LockPositionZ);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X, rigidbody.LockRotationX);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y, rigidbody.LockRotationY);
		dynamicActor->setRigidDynamicLockFlag(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z, rigidbody.LockRotationZ);

		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, rigidbody.CollisionDetection == CollisionDetectionType::Continuous);
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, rigidbody.CollisionDetection == CollisionDetectionType::ContinuousSpeculative);

		physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidbody.Mass);
	}

	void Physics::DestroyPhysicsBody(Entity entity)
	{
		// Destroy the physics body if it exists

		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
		physx::PxActor* actor = static_cast<physx::PxActor*>(rigidbody.RuntimeActor);

		if (actor == nullptr)
		{
			VX_CORE_WARN("Trying to delete Physics Actor that doesn't exist!");
			return;
		}

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			physx::PxController* controller = static_cast<physx::PxController*>(entity.GetComponent<CharacterControllerComponent>().RuntimeController);
			controller->release();
		}

		// Destroy the physics body data
		PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)actor->userData;
		delete physicsBodyData;

		s_Data->PhysicsScene->removeActor(*actor);
	}

}

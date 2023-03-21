#include "vxpch.h"
#include "Physics.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Renderer/Renderer2D.h"
#include "Vortex/Physics/3D/PhysXUtilities.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"
#include "Vortex/Physics/3D/PhysicsFilterShader.h"
#include "Vortex/Physics/3D/PhysicsContactListener.h"
#include "Vortex/Physics/3D/CookingFactory.h"
#include "Vortex/Scripting/ScriptEngine.h"

#include "Vortex/Utils/Time.h"

namespace Vortex {

	struct PhysicsEngineInternalData
	{
		physx::PxDefaultAllocator DefaultAllocator;
		physx::PxDefaultErrorCallback ErrorCallback;
		physx::PxFoundation* Foundation = nullptr;
		physx::PxPhysics* PhysXSDK = nullptr;
		physx::PxDefaultCpuDispatcher* Dispatcher = nullptr;
		physx::PxControllerManager* ControllerManager = nullptr;
		physx::PxScene* PhysicsScene = nullptr;
		physx::PxTolerancesScale TolerancesScale;

#ifndef VX_DIST
		physx::PxSimulationStatistics SimulationStats;
#endif
		
		PhysicsContactListener ContactListener;

		// TODO substep through the simulation
		struct SubstepInfo
		{
			float SubstepSize = 0.0f;
			float Accumulator = 0.0f;
			uint32_t NumSubsteps = 0;
			inline static constexpr uint32_t MaxSubsteps = 8;
		} SubstepInfo;

		Scene* ContextScene = nullptr;

		std::unordered_map<UUID, physx::PxRigidActor*> ActiveActors;
		std::unordered_map<UUID, physx::PxController*> ActiveControllers;
		std::unordered_map<UUID, physx::PxFixedJoint*> ActiveFixedJoints;

		using EntityColliderMap = std::unordered_map<UUID, std::vector<SharedReference<ColliderShape>>>;
		EntityColliderMap EntityColliders;

		//                                                                               first - linear force, second - angular force
		using LastReportedFixedJointForcesMap = std::unordered_map<physx::PxFixedJoint*, std::pair<Math::vec3, Math::vec3>>;
		LastReportedFixedJointForcesMap LastReportedJointForces;

		std::unordered_map<UUID, PhysicsBodyData*> PhysicsBodyData;
		std::unordered_map<UUID, ConstrainedJointData*> ConstrainedJointData;

		constexpr static float FixedTimeStep = 1.0f / 100.0f;
		Math::vec3 SceneGravity = Math::vec3(0.0f, -9.81f, 0.0f);
		uint32_t PositionSolverIterations = 8;
		uint32_t VelocitySolverIterations = 2;
	};

	static PhysicsEngineInternalData* s_Data = nullptr;

	void Physics::Init()
	{
		InitPhysicsSDKInternal();
	}

	void Physics::Shutdown()
	{
		ShutdownPhysicsSDKInternal();
	}

	void Physics::OnSimulationStart(Scene* contextScene)
	{
		InitPhysicsSceneInternal();

		s_Data->ContextScene = contextScene;

		TraverseSceneForUninitializedActors();
	}

	void Physics::OnSimulationUpdate(TimeStep delta)
	{
		TraverseSceneForUninitializedActors();

		SimulationStep();

		UpdateActors();
		UpdateControllers();
		UpdateFixedJoints();

#ifndef VX_DIST
		s_Data->PhysicsScene->getSimulationStatistics(s_Data->SimulationStats);
#endif
	}

	void Physics::OnSimulationStop(Scene* contextScene)
	{
		std::vector<UUID> actorsToDestroy;

		for (const auto& [entityUUID, fixedJoint] : s_Data->ActiveFixedJoints)
			actorsToDestroy.push_back(entityUUID);

		for (const auto& [entityUUID, characterController] : s_Data->ActiveControllers)
			actorsToDestroy.push_back(entityUUID);

		for (const auto& [entityUUID, actor] : s_Data->ActiveActors)
			actorsToDestroy.push_back(entityUUID);

		std::unique(actorsToDestroy.begin(), actorsToDestroy.end());

		for (const auto& entityUUID : actorsToDestroy)
		{
			Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);

			if (!entity)
				continue;

			DestroyPhysicsActor(entity);
		}

		ShutdownPhysicsSceneInternal();
	}

	void Physics::CreatePhysicsActor(Entity entity)
	{
		physx::PxRigidActor* actor = nullptr;

		const TransformComponent& transform = entity.GetTransform();
		RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

		if (rigidbody.Type == RigidBodyType::Static)
		{
			actor = s_Data->PhysXSDK->createRigidStatic(ToPhysXTransform(transform));
		}
		else if (rigidbody.Type == RigidBodyType::Dynamic)
		{
			physx::PxRigidDynamic* dynamicActor = s_Data->PhysXSDK->createRigidDynamic(ToPhysXTransform(transform));
			UpdateDynamicActorProperties(rigidbody, dynamicActor);
			actor = dynamicActor;
		}

		VX_CORE_ASSERT(actor != nullptr, "Failed to create Physics Actor!");
		rigidbody.RuntimeActor = (void*)actor;

		UUID entityUUID = entity.GetUUID();
		PhysicsBodyData* physicsBodyData = new PhysicsBodyData();
		physicsBodyData->EntityUUID = entityUUID;
		physicsBodyData->ContextScene = entity.GetContextScene();
		actor->userData = physicsBodyData;

		s_Data->PhysicsBodyData[entityUUID] = physicsBodyData;
		s_Data->ActiveActors[entityUUID] = actor;

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			VX_CORE_ASSERT(!s_Data->ActiveControllers.contains(entity.GetUUID()), "Entities cannot have multiple controllers!");

			physx::PxController* controller = CreateController(entity);
			s_Data->ActiveControllers[entityUUID] = controller;
			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.RuntimeController = (void*)controller;

			SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);

			return;
		}

		CreateCollider(entity);

		// Set Filters
		if (rigidbody.Type == RigidBodyType::Static)
			SetCollisionFilters(actor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);
		else if (rigidbody.Type == RigidBodyType::Dynamic)
			SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);

		s_Data->PhysicsScene->addActor(*actor);
	}

	void Physics::ReCreateActor(Entity entity)
	{
		DestroyPhysicsActor(entity);
		CreatePhysicsActor(entity);
	}

	void Physics::DestroyPhysicsActor(Entity entity)
	{
		if (!entity.HasComponent<RigidBodyComponent>())
			return;

		UUID entityUUID = entity.GetUUID();

		DestroyColliderShapesInternal(entityUUID);

		DestroyFixedJointInternal(entityUUID);
		DestroyCharacterControllerInternal(entityUUID);
		DestroyPhysicsActorInternal(entityUUID);

		DestroyPhysicsBodyDataInternal(entityUUID);
		DestroyConstrainedJointDataInternal(entityUUID);
	}

	void Physics::WakeUpActor(Entity entity)
	{
		if (!entity.HasComponent<RigidBodyComponent>())
		{
			VX_CONSOLE_LOG_WARN("Cannot wake up Entity without RigidBody Component '{}', '{}'", entity.GetName(), entity.GetUUID());
			return;
		}

		const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();
		
		if (rigidbody.Type != RigidBodyType::Dynamic)
		{
			VX_CONSOLE_LOG_WARN("Cannot wake up Static Actor '{}', '{}'", entity.GetName(), entity.GetUUID());
			return;
		}

		physx::PxActor* actor = (physx::PxActor*)rigidbody.RuntimeActor;

		if (physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>())
		{
			dynamicActor->wakeUp();
		}
	}

	void Physics::WakeUpActors()
	{
		VX_PROFILE_FUNCTION();

		if (!s_Data->PhysicsScene || !s_Data->ContextScene)
			return;

		physx::PxActorTypeFlags flags = physx::PxActorTypeFlag::eRIGID_DYNAMIC;
		uint32_t count = s_Data->PhysicsScene->getNbActors(flags);

		physx::PxActor** buffer = new physx::PxActor*[count];
		s_Data->PhysicsScene->getActors(flags, buffer, count);

		for (uint32_t i = 0; i < count; i++)
		{
			physx::PxRigidDynamic* actor = buffer[i]->is<physx::PxRigidDynamic>();

			const bool gravityDisabled = actor->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY;
			const bool isAwake = !actor->isSleeping();

			if (gravityDisabled || isAwake)
				continue;

			actor->wakeUp();
		}

		delete[] buffer;
	}

	bool Physics::Raycast(const Math::vec3& origin, const Math::vec3& direction, float maxDistance, RaycastHit* outHitInfo)
	{
		physx::PxRaycastBuffer hitInfo;
		bool result = s_Data->PhysicsScene->raycast(ToPhysXVector(origin), ToPhysXVector(Math::Normalize(direction)), maxDistance, hitInfo);

		if (result)
		{
			void* userData = hitInfo.block.actor->userData;

			if (!userData)
			{
				*outHitInfo = RaycastHit();
				return false;
			}

			PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)userData;
			UUID entityUUID = physicsBodyData->EntityUUID;

			// Call Hit Entity's OnRaycastCollision Method
			Scene* contextScene = ScriptEngine::GetContextScene();
			VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
			Entity hitEntity = contextScene->TryGetEntityWithUUID(entityUUID);
			VX_CORE_ASSERT(hitEntity, "Entity UUID was Invalid!");

			if (hitEntity.HasComponent<ScriptComponent>())
			{
				const ScriptComponent& scriptComponent = hitEntity.GetComponent<ScriptComponent>();

				if (ScriptEngine::EntityClassExists(scriptComponent.ClassName))
				{
					ScriptEngine::OnRaycastCollisionEntity(hitEntity);
				}
			}

			outHitInfo->EntityID = entityUUID;
			outHitInfo->Position = FromPhysXVector(hitInfo.block.position);
			outHitInfo->Normal = FromPhysXVector(hitInfo.block.normal);
			outHitInfo->Distance = hitInfo.block.distance;
		}

		return result;
	}

	bool Physics::IsConstraintBroken(UUID entityUUID)
	{
		if (s_Data->ActiveFixedJoints.contains(entityUUID))
		{
			physx::PxFixedJoint* fixedJoint = s_Data->ActiveFixedJoints[entityUUID];
			return fixedJoint->getConstraintFlags() & physx::PxConstraintFlag::eBROKEN;
		}

		return false;
	}

	void Physics::BreakJoint(UUID entityUUID)
	{
		if (s_Data->ActiveFixedJoints.contains(entityUUID))
		{
			physx::PxFixedJoint* fixedJoint = s_Data->ActiveFixedJoints[entityUUID];
			fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eBROKEN, true);
		}
	}

    void Physics::OnCharacterControllerUpdateRuntime(UUID entityUUID, const Math::vec3& displacement)
    {
		Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);

		VX_CORE_ASSERT(entity, "Invalid Entity UUID!");
		VX_CORE_ASSERT(entity.HasComponent<CharacterControllerComponent>(), "Entity doesn't have Character Controller!");

		CharacterControllerComponent& characterControllerComponent = entity.GetComponent<CharacterControllerComponent>();

		VX_CORE_ASSERT(s_Data->ActiveControllers.contains(entityUUID), "Invalid Controller!");
		physx::PxControllerFilters filters; // TODO
		physx::PxController* controller = s_Data->ActiveControllers[entityUUID];

		auto gravity = Physics::GetPhysicsSceneGravity();

		if (!characterControllerComponent.DisableGravity)
		{
			characterControllerComponent.SpeedDown -= gravity.y * Time::GetDeltaTime();
		}

		Math::vec3 upDirection = FromPhysXVector(controller->getUpDirection());
		Math::vec3 movement = (displacement - upDirection) * (characterControllerComponent.SpeedDown * Time::GetDeltaTime());

		controller->move(ToPhysXVector(movement), 0.0f, Time::GetDeltaTime(), filters);
		entity.GetTransform().Translation = FromPhysXExtendedVector(controller->getPosition());

		physx::PxControllerState state;
		controller->getState(state);

		// test if grounded
		if (state.collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
		{
			characterControllerComponent.SpeedDown = gravity.y * 0.01f;
		}
    }

	void Physics::SimulationStep()
	{
		s_Data->PhysicsScene->simulate(s_Data->FixedTimeStep);
		s_Data->PhysicsScene->fetchResults(true);
		s_Data->PhysicsScene->setGravity(ToPhysXVector(s_Data->SceneGravity));
	}

	void Physics::UpdateActors()
	{
		for (const auto& [entityUUID, actor] : s_Data->ActiveActors)
		{
			Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);
			auto& transform = entity.GetTransform();

			const auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.Type == RigidBodyType::Dynamic)
			{
				physx::PxRigidDynamic* dynamicActor = actor->is<physx::PxRigidDynamic>();

				entity.SetTransform(FromPhysXTransform(dynamicActor->getGlobalPose()) * Math::Scale(transform.Scale));

				UpdateDynamicActorProperties(rigidbody, dynamicActor);
			}
			else if (rigidbody.Type == RigidBodyType::Static)
			{
				// Synchronize with entity Transform
				actor->setGlobalPose(ToPhysXTransform(transform));
			}
		}
	}

	void Physics::UpdateControllers()
	{
		for (const auto& [entityUUID, characterController] : s_Data->ActiveControllers)
		{
			Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);
			const CharacterControllerComponent& characterControllerComponent = entity.GetComponent<CharacterControllerComponent>();
			auto& transform = entity.GetTransform();

			Math::vec3 position = FromPhysXExtendedVector(characterController->getPosition());

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

			characterController->setStepOffset(characterControllerComponent.StepOffset);
			characterController->setSlopeLimit(Math::Deg2Rad(characterControllerComponent.SlopeLimitDegrees));

			transform.Translation = position;
		}
	}

	void Physics::UpdateFixedJoints()
	{
		for (const auto& [entityUUID, fixedJoint] : s_Data->ActiveFixedJoints)
		{
			physx::PxVec3 linear(0.0f), angular(0.0f);
			physx::PxFixedJoint* fixedJoint = s_Data->ActiveFixedJoints[entityUUID];
			fixedJoint->getConstraint()->getForce(linear, angular);

			Math::vec3 linearForce = FromPhysXVector(linear);
			Math::vec3 angularForce = FromPhysXVector(angular);
			s_Data->LastReportedJointForces[fixedJoint] = std::make_pair(linearForce, angularForce);
		}
	}

	void Physics::CreateCollider(Entity entity)
	{
		const auto& transform = entity.GetTransform();
		physx::PxRigidActor* actor = (physx::PxRigidActor*)entity.GetComponent<RigidBodyComponent>().RuntimeActor;

		if (entity.HasComponent<BoxColliderComponent>())
		{
			AddColliderShape(entity, actor, ColliderType::Box);
		}
		else if (entity.HasComponent<SphereColliderComponent>())
		{
			AddColliderShape(entity, actor, ColliderType::Sphere);
		}
		else if (entity.HasComponent<CapsuleColliderComponent>())
		{
			AddColliderShape(entity, actor, ColliderType::Capsule);
		}
		else if (entity.HasComponent<MeshColliderComponent>())
		{
			// TODO
			//AddColliderShape(entity, actor, ColliderType::TriangleMesh);
		}
	}

	void Physics::AddColliderShape(Entity entity, physx::PxRigidActor* actor, ColliderType type)
	{
		UUID entityUUID = entity.GetUUID();

		if (!s_Data->EntityColliders.contains(entityUUID))
		{
			s_Data->EntityColliders[entityUUID] = std::vector<SharedReference<ColliderShape>>();
		}

		switch (type)
		{
			case ColliderType::Box:
			{
				BoxColliderComponent& boxCollider = entity.GetComponent<BoxColliderComponent>();
				s_Data->EntityColliders[entityUUID].push_back(SharedReference<BoxColliderShape>::Create(boxCollider, *actor, entity));
				break;
			}
			case ColliderType::Sphere:
			{
				SphereColliderComponent& sphereCollider = entity.GetComponent<SphereColliderComponent>();
				s_Data->EntityColliders[entityUUID].push_back(SharedReference<SphereColliderShape>::Create(sphereCollider, *actor, entity));
				break;
			}
			case ColliderType::Capsule:
			{
				CapsuleColliderComponent& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();
				s_Data->EntityColliders[entityUUID].push_back(SharedReference<CapsuleColliderShape>::Create(capsuleCollider, *actor, entity));
				break;
			}
			case ColliderType::ConvexMesh:
			{
				MeshColliderComponent& convexMeshCollider = entity.GetComponent<MeshColliderComponent>();
				s_Data->EntityColliders[entityUUID].push_back(SharedReference<ConvexMeshShape>::Create(convexMeshCollider, *actor, entity));
				break;
			}
			case ColliderType::TriangleMesh:
			{
				MeshColliderComponent& triangleMeshCollider = entity.GetComponent<MeshColliderComponent>();
				s_Data->EntityColliders[entityUUID].push_back(SharedReference<TriangleMeshShape>::Create(triangleMeshCollider, *actor, entity));
				break;
			}
		}
	}

	physx::PxMaterial* Physics::AddControllerColliderShape(Entity entity, physx::PxRigidActor* actor, ColliderType type)
	{
		AddColliderShape(entity, actor, type);

		return s_Data->EntityColliders[entity.GetUUID()].back()->GetMaterial();
	}

	void Physics::CreateFixedJoint(Entity entity)
	{
		VX_CORE_ASSERT(entity.HasComponent<FixedJointComponent>(), "Entity doesn't have Fixed Joint Component");

		if (!entity)
		{
			return;
		}

		const FixedJointComponent& fixedJointComponent = entity.GetComponent<FixedJointComponent>();
		UUID connectedEntityUUID = fixedJointComponent.ConnectedEntity;
		Entity connectedEntity = s_Data->ContextScene->TryGetEntityWithUUID(connectedEntityUUID);
		VX_CORE_ASSERT(connectedEntity, "Connected Entity was Invalid!");

		if (!connectedEntity)
		{
			return;
		}

		physx::PxRigidActor* actor0 = GetActor(entity);
		physx::PxRigidActor* actor1 = GetActor(connectedEntity);

		if (!actor0 || !actor1)
		{
			VX_CONSOLE_LOG_ERROR("Unable to create Fixed Joint with entities '{}', '{}'", entity.GetName(), connectedEntity.GetName());
			return;
		}

		physx::PxTransform localFrame0 = Utils::GetLocalFrame(actor0);
		physx::PxTransform localFrame1 = Utils::GetLocalFrame(actor1);

		VX_CORE_ASSERT(!s_Data->ActiveFixedJoints.contains(entity.GetUUID()), "Entities can only have one Fixed Joint!");

		physx::PxFixedJoint* fixedJoint = physx::PxFixedJointCreate(*s_Data->PhysXSDK, actor0, localFrame0, actor1, localFrame1);
		s_Data->ActiveFixedJoints[entity.GetUUID()] = fixedJoint;

		ConstrainedJointData* jointData = new ConstrainedJointData();
		jointData->EntityUUID = entity.GetUUID();
		fixedJoint->userData = jointData;

		s_Data->ConstrainedJointData[entity.GetUUID()] = jointData;

		fixedJoint->setBreakForce(fixedJointComponent.BreakForce, fixedJointComponent.BreakTorque);
		fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, fixedJointComponent.EnableCollision);
		fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eDISABLE_PREPROCESSING, !fixedJointComponent.EnablePreProcessing);
	}

	physx::PxController* Physics::CreateController(Entity entity)
	{
		const auto& transform = entity.GetTransform();
		CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();

		physx::PxController* controller = nullptr;

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			const auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();

			physx::PxMaterial* material = AddControllerColliderShape(entity, controller->getActor(), ColliderType::Capsule);

			const float radiusScale = Math::Max(transform.Scale.x, transform.Scale.y);

			physx::PxCapsuleControllerDesc desc;
			desc.position = ToPhysXExtendedVector(transform.Translation + capsuleCollider.Offset);
			desc.height = capsuleCollider.Height * transform.Scale.y;
			desc.radius = capsuleCollider.Radius * radiusScale;
			desc.nonWalkableMode = (physx::PxControllerNonWalkableMode::Enum)characterController.NonWalkMode;
			desc.climbingMode = (physx::PxCapsuleClimbingMode::Enum)characterController.ClimbMode;
			desc.slopeLimit = Math::Max(0.0f, cosf(Math::Deg2Rad(characterController.SlopeLimitDegrees)));
			desc.stepOffset = characterController.StepOffset;
			desc.contactOffset = characterController.ContactOffset;
			desc.material = material;
			desc.upDirection = { 0.0f, 1.0f, 0.0f };

			controller = s_Data->ControllerManager->createController(desc);
		}
		else if (entity.HasComponent<BoxColliderComponent>())
		{
			const auto& boxCollider = entity.GetComponent<BoxColliderComponent>();

			physx::PxMaterial* material = AddControllerColliderShape(entity, controller->getActor(), ColliderType::Box);

			physx::PxBoxControllerDesc desc;
			desc.position = ToPhysXExtendedVector(transform.Translation + boxCollider.Offset);
			desc.halfHeight = (boxCollider.HalfSize.y * transform.Scale.y);
			desc.halfSideExtent = (boxCollider.HalfSize.x * transform.Scale.x);
			desc.halfForwardExtent = (boxCollider.HalfSize.z * transform.Scale.z);
			desc.nonWalkableMode = (physx::PxControllerNonWalkableMode::Enum)characterController.NonWalkMode;
			desc.slopeLimit = Math::Max(0.0f, cosf(Math::Deg2Rad(characterController.SlopeLimitDegrees)));
			desc.stepOffset = characterController.StepOffset;
			desc.contactOffset = characterController.ContactOffset;
			desc.material = material;
			desc.upDirection = { 0.0f, 1.0f, 0.0f };

			controller = s_Data->ControllerManager->createController(desc);
		}

		return controller;
	}

	void Physics::SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask)
	{
		physx::PxFilterData filterData;
		filterData.word0 = filterGroup; // word0 = own ID
		filterData.word1 = filterMask;  // word1 = ID mask to filter pairs that trigger a contact callback

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

	void Physics::UpdateDynamicActorProperties(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor)
	{
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, rigidbody.IsKinematic);
		dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, rigidbody.DisableGravity);

		dynamicActor->setSolverIterationCounts(Physics::GetPhysicsScenePositionIterations(), Physics::GetPhysicsSceneVelocityIterations());

		dynamicActor->setMass(rigidbody.Mass);

		if (rigidbody.LinearVelocity != Math::vec3(0.0f))
		{
			Math::vec3 linearVelocity = rigidbody.LinearVelocity;
			physx::PxVec3 actorVelocity = dynamicActor->getLinearVelocity();
			// If any component of the vector is 0 just use the actors velocity
			Utils::ReplaceInconsistentVectorAxis(linearVelocity, actorVelocity);

			dynamicActor->setLinearVelocity(ToPhysXVector(linearVelocity));
		}

		dynamicActor->setMaxLinearVelocity(rigidbody.MaxLinearVelocity);
		dynamicActor->setLinearDamping(rigidbody.LinearDrag);

		if (rigidbody.AngularVelocity != Math::vec3(0.0f))
		{
			Math::vec3 angularVelocity = rigidbody.AngularVelocity;
			physx::PxVec3 actorVelocity = dynamicActor->getAngularVelocity();
			// If any component of the vector is 0 just use the actors velocity
			Utils::ReplaceInconsistentVectorAxis(angularVelocity, actorVelocity);

			dynamicActor->setAngularVelocity(ToPhysXVector(angularVelocity));
		}

		dynamicActor->setMaxAngularVelocity(rigidbody.MaxAngularVelocity);
		dynamicActor->setAngularDamping(rigidbody.AngularDrag);

		uint8_t lockFlags = rigidbody.LockFlags & (uint8_t)ActorLockFlag::TranslationX |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::TranslationY |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::TranslationZ |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::RotationX |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::RotationY |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::RotationZ;

		dynamicActor->setRigidDynamicLockFlags((physx::PxRigidDynamicLockFlags)lockFlags);

		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, rigidbody.CollisionDetection == CollisionDetectionType::Continuous);
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, rigidbody.CollisionDetection == CollisionDetectionType::ContinuousSpeculative);

		physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidbody.Mass);
	}

	void Physics::TraverseSceneForUninitializedActors()
	{
		auto view = s_Data->ContextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent>();

		for (const auto e : view)
		{
			Entity entity{ e, s_Data->ContextScene };
			const RigidBodyComponent& rigidbody = entity.GetComponent<RigidBodyComponent>();

			if (rigidbody.RuntimeActor || s_Data->ActiveActors.contains(entity.GetUUID()))
				continue;

			CreatePhysicsActor(entity);
		}

		auto characterControllerView = s_Data->ContextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent, CharacterControllerComponent>();

		for (const auto e : characterControllerView)
		{
			Entity entity{ e, s_Data->ContextScene };
			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();

			if (characterController.RuntimeController || s_Data->ActiveControllers.contains(entity.GetUUID()))
				continue;

			CreatePhysicsActor(entity);
		}

		auto fixedJointView = s_Data->ContextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent, FixedJointComponent>();

		for (const auto e : fixedJointView)
		{
			Entity entity{ e, s_Data->ContextScene };

			if (s_Data->ActiveFixedJoints.contains(entity.GetUUID()))
				continue;

			CreateFixedJoint(entity);
		}
	}

	void Physics::DestroyFixedJointInternal(UUID entityUUID)
	{
		if (!s_Data->ActiveFixedJoints.contains(entityUUID))
			return;

		Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);
		FixedJointComponent& fixedJoint = entity.GetComponent<FixedJointComponent>();
		fixedJoint.ConnectedEntity = 0;
		s_Data->ActiveFixedJoints[entityUUID]->release();
		s_Data->ActiveFixedJoints.erase(entityUUID);
	}

	void Physics::DestroyCharacterControllerInternal(UUID entityUUID)
	{
		if (!s_Data->ActiveControllers.contains(entityUUID))
			return;

		Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);
		CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
		characterController.RuntimeController = nullptr;
		s_Data->ActiveControllers[entityUUID]->release();
		s_Data->ActiveControllers.erase(entityUUID);
	}

	void Physics::DestroyPhysicsActorInternal(UUID entityUUID)
	{
		if (!s_Data->ActiveActors.contains(entityUUID))
			return;

		Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);
		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
		physx::PxRigidActor* actor = s_Data->ActiveActors[entityUUID];
		s_Data->PhysicsScene->removeActor(*actor);
		actor->release();
		rigidbody.RuntimeActor = nullptr;
		s_Data->ActiveActors.erase(entityUUID);
	}

	void Physics::DestroyColliderShapesInternal(UUID entityUUID)
	{
		if (!s_Data->EntityColliders.contains(entityUUID))
			return;

		auto& colliderShapes = s_Data->EntityColliders[entityUUID];

		for (auto& colliderShape : colliderShapes)
		{
			physx::PxRigidActor* actor = GetActor(entityUUID);
			if (!actor)
				continue;

			colliderShape->DetachFromActor(actor);
		}

		colliderShapes.clear();
		s_Data->EntityColliders.erase(entityUUID);
	}

	void Physics::DestroyPhysicsBodyDataInternal(UUID entityUUID)
	{
		if (!s_Data->PhysicsBodyData.contains(entityUUID))
			return;

		PhysicsBodyData* physicsBodyData = s_Data->PhysicsBodyData[entityUUID];
		if (!physicsBodyData)
			return;

		delete physicsBodyData;
		s_Data->PhysicsBodyData.erase(entityUUID);
	}

	void Physics::DestroyConstrainedJointDataInternal(UUID entityUUID)
	{
		if (!s_Data->ConstrainedJointData.contains(entityUUID))
			return;

		ConstrainedJointData* jointData = s_Data->ConstrainedJointData[entityUUID];
		if (!jointData)
			return;

		delete jointData;
		s_Data->ConstrainedJointData.erase(entityUUID);
	}

	void Physics::InitPhysicsSDKInternal()
	{
		s_Data = new PhysicsEngineInternalData();

		// Create the scene with the description
		s_Data->Foundation = PxCreateFoundation(PX_PHYSICS_VERSION, s_Data->DefaultAllocator, s_Data->ErrorCallback);
		VX_CORE_ASSERT(s_Data->Foundation, "Failed to create Physics Scene Foundation!");

		s_Data->TolerancesScale = physx::PxTolerancesScale();
		s_Data->TolerancesScale.length = 1.0; // Typical length of an object
		s_Data->TolerancesScale.speed = 100.0f; // Typical speed of an object, gravity * speed is a reasonable choice
		s_Data->PhysXSDK = PxCreatePhysics(PX_PHYSICS_VERSION, *s_Data->Foundation, s_Data->TolerancesScale, true, nullptr);

		bool extentionsLoaded = PxInitExtensions(*s_Data->PhysXSDK, nullptr);
		VX_CORE_ASSERT(extentionsLoaded, "Failed to initialize PhysX Extensions");

		s_Data->Dispatcher = physx::PxDefaultCpuDispatcherCreate(1);

		CookingFactory::Init();
	}

	void Physics::InitPhysicsSceneInternal()
	{
		physx::PxSceneDesc sceneDescription = physx::PxSceneDesc(s_Data->TolerancesScale);
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDescription.gravity = ToPhysXVector(s_Data->SceneGravity);

		SharedReference<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		sceneDescription.broadPhaseType = Utils::VortexBroadphaseTypeToPhysXBroadphaseType(projectProps.PhysicsProps.BroadphaseModel);
		sceneDescription.frictionType = Utils::VortexFrictionTypeToPhysXFrictionType(projectProps.PhysicsProps.FrictionModel);

		sceneDescription.cpuDispatcher = s_Data->Dispatcher;
		sceneDescription.filterShader = PhysicsFilterShader::FilterShader;
		sceneDescription.simulationEventCallback = &s_Data->ContactListener;

		s_Data->PhysicsScene = s_Data->PhysXSDK->createScene(sceneDescription);
		s_Data->ControllerManager = PxCreateControllerManager(*s_Data->PhysicsScene);
	}

	void Physics::ShutdownPhysicsSDKInternal()
	{
		CookingFactory::Shutdown();

		s_Data->Dispatcher->release();
		PxCloseExtensions();
		s_Data->PhysXSDK->release();
		s_Data->Foundation->release();

		delete s_Data;
		s_Data = nullptr;
	}

	void Physics::ShutdownPhysicsSceneInternal()
	{
		s_Data->ControllerManager->release();
		s_Data->ControllerManager = nullptr;

		s_Data->PhysicsScene->release();
		s_Data->PhysicsScene = nullptr;

		s_Data->ContextScene = nullptr;

		s_Data->EntityColliders.clear();

		s_Data->ActiveFixedJoints.clear();
		s_Data->ActiveControllers.clear();
		s_Data->ActiveActors.clear();

		s_Data->ConstrainedJointData.clear();
		s_Data->PhysicsBodyData.clear();
	}

	const std::unordered_map<UUID, physx::PxRigidActor*>& Physics::GetActors()
	{
		return s_Data->ActiveActors;
	}

	const std::unordered_map<UUID, physx::PxController*>& Physics::GetControllers()
	{
		return s_Data->ActiveControllers;
	}

	const std::unordered_map<UUID, physx::PxFixedJoint*> Physics::GetFixedJoints()
	{
		return s_Data->ActiveFixedJoints;
	}

	physx::PxRigidActor* Physics::GetActor(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_Data->ActiveActors.contains(entityUUID), "Entity was not found in active actors map!");

		if (s_Data->ActiveActors.contains(entityUUID))
		{
			return s_Data->ActiveActors[entityUUID];
		}

		return nullptr;
	}

	const std::vector<SharedReference<ColliderShape>>& Physics::GetEntityColliders(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_Data->EntityColliders.contains(entityUUID), "Entity doesn't have any colliders!");
		return s_Data->EntityColliders.at(entityUUID);
	}

	physx::PxController* Physics::GetController(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_Data->ActiveControllers.contains(entityUUID), "Entity was not found in active controllers map!");

		if (s_Data->ActiveControllers.contains(entityUUID))
		{
			return s_Data->ActiveControllers[entityUUID];
		}

		return nullptr;
	}

	physx::PxFixedJoint* Physics::GetFixedJoint(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_Data->ActiveFixedJoints.contains(entityUUID), "Entity was not found in active fixed joint map!");

		if (s_Data->ActiveFixedJoints.contains(entityUUID))
		{
			return s_Data->ActiveFixedJoints[entityUUID];
		}

		return nullptr;
	}

	const std::pair<Math::vec3, Math::vec3>& Physics::GetLastReportedFixedJointForces(physx::PxFixedJoint* fixedJoint)
	{
		VX_CORE_ASSERT(s_Data->LastReportedJointForces.contains(fixedJoint), "Fixed Joint was not found in last reported forces map!");

		if (s_Data->LastReportedJointForces.contains(fixedJoint))
		{
			return s_Data->LastReportedJointForces[fixedJoint];
		}

		return {};
	}

	const PhysicsBodyData* Physics::GetPhysicsBodyData(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_Data->PhysicsBodyData.contains(entityUUID), "Entity was not found in physics body data map!");

		if (s_Data->PhysicsBodyData.contains(entityUUID))
		{
			return s_Data->PhysicsBodyData.at(entityUUID);
		}

		return nullptr;
	}

	const ConstrainedJointData* Physics::GetConstrainedJointData(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_Data->ConstrainedJointData.contains(entityUUID), "Entity was not found in constrained joint data map!");

		if (s_Data->ConstrainedJointData.contains(entityUUID))
		{
			return s_Data->ConstrainedJointData.at(entityUUID);
		}

		return nullptr;
	}

	Scene* Physics::GetContextScene()
	{
		return s_Data->ContextScene;
	}

	physx::PxScene* Physics::GetPhysicsScene()
	{
		return s_Data->PhysicsScene;
	}

#ifndef VX_DIST

	physx::PxSimulationStatistics* Physics::GetSimulationStatistics()
	{
		return &s_Data->SimulationStats;
	}

#endif

	uint32_t Physics::GetPhysicsScenePositionIterations()
	{
		return s_Data->PositionSolverIterations;
	}

	void Physics::SetPhysicsScenePositionIterations(uint32_t positionIterations)
	{
		s_Data->PositionSolverIterations = positionIterations;
	}

	uint32_t Physics::GetPhysicsSceneVelocityIterations()
	{
		return s_Data->VelocitySolverIterations;
	}

	void Physics::SetPhysicsSceneVelocityIterations(uint32_t veloctiyIterations)
	{
		s_Data->VelocitySolverIterations = veloctiyIterations;
	}

	Math::vec3 Physics::GetPhysicsSceneGravity()
	{
		return s_Data->SceneGravity;
	}

	void Physics::SetPhysicsSceneGravity(const Math::vec3& gravity)
	{
		s_Data->SceneGravity = gravity;
	}

	physx::PxPhysics* Physics::GetPhysicsSDK()
	{
		return s_Data->PhysXSDK;
	}

	physx::PxTolerancesScale* Physics::GetTolerancesScale()
	{
		return &s_Data->TolerancesScale;
	}

	physx::PxFoundation* Physics::GetFoundation()
	{
		return s_Data->Foundation;
	}

}

#include "vxpch.h"
#include "Physics.h"

#include "Vortex/Core/Application.h"

#include "Vortex/Module/Module.h"

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

		using ActorColliderMap = std::unordered_map<UUID, std::vector<SharedReference<ColliderShape>>>;
		ActorColliderMap ActorColliders;

		//                                                                               first - linear force, second - angular force
		using LastReportedFixedJointForcesMap = std::unordered_map<physx::PxFixedJoint*, std::pair<Math::vec3, Math::vec3>>;
		LastReportedFixedJointForcesMap LastReportedJointForces;

		std::unordered_map<UUID, PhysicsBodyData*> PhysicsBodyData;
		std::unordered_map<UUID, ConstrainedJointData*> ConstrainedJointData;

		constexpr static float FixedTimeStep = 1.0f / 100.0f;
		Math::vec3 SceneGravity = Math::vec3(0.0f, -9.81f, 0.0f);
		uint32_t PositionSolverIterations = 8;
		uint32_t VelocitySolverIterations = 2;

		SubModule Module;
	};

	static PhysicsEngineInternalData* s_Data = nullptr;

	void Physics::Init()
	{
		InitPhysicsSDKInternal();

		SubModuleProperties moduleProps;
		moduleProps.ModuleName = "Physics";
		moduleProps.APIVersion = Version(1, 1, 0);
		moduleProps.RequiredModules = {};
		s_Data->Module.Init(moduleProps);

		Application::Get().AddModule(s_Data->Module);
	}

	void Physics::Shutdown()
	{
		ShutdownPhysicsSDKInternal();

		Application::Get().RemoveModule(s_Data->Module);
		s_Data->Module.Shutdown();

		delete s_Data;
		s_Data = nullptr;
	}

	void Physics::OnSimulationStart(Scene* contextScene)
	{
		InitPhysicsSceneInternal();

		s_Data->ContextScene = contextScene;

		InitializeUninitializedActors();
	}

	void Physics::OnSimulationUpdate(TimeStep delta)
	{
		InitializeUninitializedActors();

		RT_SimulationStep();

		RT_UpdateActors();
		RT_UpdateControllers();
		RT_UpdateJoints();

#ifndef VX_DIST
		s_Data->PhysicsScene->getSimulationStatistics(s_Data->SimulationStats);
#endif
	}

	void Physics::OnSimulationStop(Scene* contextScene)
	{
		std::vector<UUID> actorsToDestroy;

		for (const auto& [actorUUID, fixedJoint] : s_Data->ActiveFixedJoints)
		{
			actorsToDestroy.emplace_back(actorUUID);
		}

		for (const auto& [actorUUID, characterController] : s_Data->ActiveControllers)
		{
			actorsToDestroy.emplace_back(actorUUID);
		}

		for (const auto& [actorUUID, actor] : s_Data->ActiveActors)
		{
			actorsToDestroy.emplace_back(actorUUID);
		}

		std::set<UUID> uniqueActors;

		uint32_t size = actorsToDestroy.size();
		for (uint32_t i = 0; i < size; i++)
		{
			uniqueActors.insert(actorsToDestroy[i]);
		}

		actorsToDestroy.assign(uniqueActors.begin(), uniqueActors.end());

		for (UUID uuid : actorsToDestroy)
		{
			Actor actor = contextScene->TryGetActorWithUUID(uuid);
			VX_CORE_ASSERT(actor, "Trying to destroy physics actor on invalid actor!");
			if (!actor)
				continue;

			DestroyPhysicsActor(actor);
		}

		ShutdownPhysicsSceneInternal();
	}

	void Physics::CreatePhysicsActor(Actor actor)
	{
		if (actor.HasComponent<CharacterControllerComponent>())
		{
			RT_CreateCharacterControllerInternal(actor);
		}
		else
		{
			RT_CreatePhysicsActorInternal(actor);
		}
	}

	void Physics::ReCreateActor(Actor actor)
	{
		if (!s_Data->ActiveActors.contains(actor.GetUUID()))
		{
			return;
		}

		DestroyPhysicsActor(actor);
		CreatePhysicsActor(actor);
	}

	void Physics::DestroyPhysicsActor(Actor actor)
	{
		if (!actor.HasComponent<RigidBodyComponent>())
			return;

		UUID actorUUID = actor.GetUUID();

		RT_DestroyColliderShapesInternal(actorUUID);

		RT_DestroyFixedJointInternal(actorUUID);
		RT_DestroyCharacterControllerInternal(actorUUID);
		RT_DestroyPhysicsActorInternal(actorUUID);

		RT_DestroyPhysicsBodyDataInternal(actorUUID);
		RT_DestroyConstrainedJointDataInternal(actorUUID);
	}

	void Physics::WakeUpActor(Actor actor)
	{
		if (!actor.HasComponent<RigidBodyComponent>())
		{
			VX_CONSOLE_LOG_WARN("[Physics] Trying to wake up Actor without RigidBody Component '{}', '{}'", actor.GetName(), actor.GetUUID());
			return;
		}

		const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();
		
		if (rigidbody.Type != RigidBodyType::Dynamic)
		{
			VX_CONSOLE_LOG_WARN("[Physics] Trying to wake up Static Actor '{}', '{}'", actor.GetName(), actor.GetUUID());
			return;
		}

		physx::PxActor* pxActor = (physx::PxActor*)rigidbody.RuntimeActor;

		if (physx::PxRigidDynamic* dynamicActor = pxActor->is<physx::PxRigidDynamic>())
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
		const bool result = s_Data->PhysicsScene->raycast(ToPhysXVector(origin), ToPhysXVector(Math::Normalize(direction)), maxDistance, hitInfo);

		if (!result)
			return false;
		
		void* userData = hitInfo.block.actor->userData;

		if (userData == nullptr)
		{
			*outHitInfo = RaycastHit();
			return false;
		}

		PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)userData;
		UUID actor = physicsBodyData->ActorUUID;

		outHitInfo->ActorID = actor;
		outHitInfo->Position = FromPhysXVector(hitInfo.block.position);
		outHitInfo->Normal = FromPhysXVector(hitInfo.block.normal);
		outHitInfo->Distance = hitInfo.block.distance;

		return true;
	}

	bool Physics::IsConstraintBroken(UUID actorUUID)
	{
		if (s_Data->ActiveFixedJoints.contains(actorUUID))
		{
			physx::PxFixedJoint* fixedJoint = s_Data->ActiveFixedJoints[actorUUID];
			return fixedJoint->getConstraintFlags() & physx::PxConstraintFlag::eBROKEN;
		}

		return false;
	}

	void Physics::BreakJoint(UUID actorUUID)
	{
		if (!s_Data->ActiveFixedJoints.contains(actorUUID))
		{
			VX_CORE_ASSERT(false, "Trying to break joint on invalid actor!");
			return;
		}

		physx::PxFixedJoint* fixedJoint = s_Data->ActiveFixedJoints[actorUUID];
		fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eBROKEN, true);
	}

    void Physics::OnCharacterControllerUpdateRuntime(UUID actorUUID, const Math::vec3& displacement)
    {
		Actor actor = s_Data->ContextScene->TryGetActorWithUUID(actorUUID);

		VX_CORE_ASSERT(actor, "Invalid Actor UUID!");
		VX_CORE_ASSERT(actor.HasComponent<CharacterControllerComponent>(), "Actor doesn't have Character Controller!");

		CharacterControllerComponent& characterControllerComponent = actor.GetComponent<CharacterControllerComponent>();

		VX_CORE_ASSERT(s_Data->ActiveControllers.contains(actorUUID), "Invalid Controller!");
		physx::PxControllerFilters filters; // TODO
		physx::PxController* controller = s_Data->ActiveControllers[actorUUID];

		const float gravity = Math::Length(Physics::GetPhysicsSceneGravity());

		if (!characterControllerComponent.DisableGravity)
		{
			characterControllerComponent.SpeedDown += gravity * Time::GetDeltaTime();
		}

		const Math::vec3 upDirection = FromPhysXVector(controller->getUpDirection());
		const Math::vec3 movement = displacement - upDirection * characterControllerComponent.SpeedDown * (float)Time::GetDeltaTime();

		physx::PxControllerCollisionFlags collisionFlags = controller->move(ToPhysXVector(movement), 0.0f, Time::GetDeltaTime(), filters);
		actor.GetTransform().Translation = FromPhysXExtendedVector(controller->getPosition());

		// test if grounded
		if (collisionFlags & physx::PxControllerCollisionFlag::eCOLLISION_DOWN)
		{
			characterControllerComponent.SpeedDown = gravity * 0.01f;
		}
    }

	void Physics::RT_SimulationStep()
	{
		s_Data->PhysicsScene->simulate(s_Data->FixedTimeStep);
		s_Data->PhysicsScene->fetchResults(true);
		s_Data->PhysicsScene->setGravity(ToPhysXVector(s_Data->SceneGravity));
	}

	void Physics::RT_UpdateActors()
	{
		for (const auto& [actorUUID, pxActor] : s_Data->ActiveActors)
		{
			Actor actor = s_Data->ContextScene->TryGetActorWithUUID(actorUUID);
			if (!actor)
				continue;

			const TransformComponent& transform = actor.GetTransform();
			const RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();

			switch (rigidbody.Type)
			{
				case RigidBodyType::Static:
				{
					// Synchronize with actor Transform
					pxActor->setGlobalPose(ToPhysXTransform(transform));
					break;
				}
				case RigidBodyType::Dynamic:
				{
					physx::PxRigidDynamic* dynamicActor = pxActor->is<physx::PxRigidDynamic>();
					actor.SetTransform(FromPhysXTransform(dynamicActor->getGlobalPose()) * Math::Scale(transform.Scale));
					RT_UpdateDynamicActorProperties(rigidbody, dynamicActor);
					break;
				}
			}
		}
	}

	void Physics::RT_UpdateControllers()
	{
		for (const auto& [actorUUID, characterController] : s_Data->ActiveControllers)
		{
			Actor actor = s_Data->ContextScene->TryGetActorWithUUID(actorUUID);
			if (!actor)
				continue;

			const CharacterControllerComponent& characterControllerComponent = actor.GetComponent<CharacterControllerComponent>();
			Math::vec3 position = FromPhysXExtendedVector(characterController->getPosition());

			if (actor.HasComponent<CapsuleColliderComponent>())
			{
				const CapsuleColliderComponent& capsuleCollider = actor.GetComponent<CapsuleColliderComponent>();
				position -= capsuleCollider.Offset;
			}
			else if (actor.HasComponent<BoxColliderComponent>())
			{
				const BoxColliderComponent& boxCollider = actor.GetComponent<BoxColliderComponent>();
				position -= boxCollider.Offset;
			}

			characterController->setStepOffset(characterControllerComponent.StepOffset);
			characterController->setSlopeLimit(Math::Deg2Rad(characterControllerComponent.SlopeLimitDegrees));

			actor.GetTransform().Translation = position;
		}
	}

	void Physics::RT_UpdateJoints()
	{
		for (const auto& [actorUUID, fixedJoint] : s_Data->ActiveFixedJoints)
		{
			Actor actor = s_Data->ContextScene->TryGetActorWithUUID(actorUUID);
			if (!actor)
				continue;

			const physx::PxFixedJoint* fixedJoint = s_Data->ActiveFixedJoints[actorUUID];
			physx::PxVec3 linear(0.0f);
			physx::PxVec3 angular(0.0f);
			fixedJoint->getConstraint()->getForce(linear, angular);

			const Math::vec3 linearForce = FromPhysXVector(linear);
			const Math::vec3 angularForce = FromPhysXVector(angular);
			const std::pair<Math::vec3, Math::vec3> lastReportedForces = std::make_pair(linearForce, angularForce);
			s_Data->LastReportedJointForces[(physx::PxFixedJoint*)fixedJoint] = lastReportedForces;
		}
	}

	void Physics::RT_RegisterPhysicsActor(Actor actor, physx::PxRigidActor* pxActor)
	{
		UUID actorUUID = actor.GetUUID();
		PhysicsBodyData* physicsBodyData = new PhysicsBodyData();
		physicsBodyData->ActorUUID = actorUUID;
		physicsBodyData->ContextScene = actor.GetContextScene();
		pxActor->userData = physicsBodyData;

		VX_CORE_ASSERT(!s_Data->PhysicsBodyData.contains(actorUUID), "only one rigid actor allowed per actor!");
		VX_CORE_ASSERT(!s_Data->ActiveActors.contains(actorUUID), "only one rigid actor allowed per actor!");

		s_Data->PhysicsBodyData[actorUUID] = physicsBodyData;
		s_Data->ActiveActors[actorUUID] = pxActor;
	}

	physx::PxRigidActor* Physics::RT_CreateRigidActor(Actor actor, RigidBodyComponent& rigidbody)
	{
		physx::PxRigidActor* pxActor = nullptr;
		const TransformComponent& transform = actor.GetTransform();

		switch (rigidbody.Type)
		{
			case RigidBodyType::Static:
			{
				physx::PxRigidActor* rigidActor = s_Data->PhysXSDK->createRigidStatic(ToPhysXTransform(transform));
				pxActor = rigidActor;
				break;
			}
			case RigidBodyType::Dynamic:
			{
				physx::PxRigidDynamic* dynamicActor = s_Data->PhysXSDK->createRigidDynamic(ToPhysXTransform(transform));
				RT_UpdateDynamicActorProperties(rigidbody, dynamicActor);
				pxActor = dynamicActor;
				break;
			}
		}

		VX_CORE_ASSERT(pxActor != nullptr, "Failed to create Physics Actor!");
		rigidbody.RuntimeActor = (void*)pxActor;

		RT_RegisterPhysicsActor(actor, pxActor);

		return pxActor;
	}

	void Physics::RT_CreatePhysicsActorInternal(Actor actor)
	{
		VX_CORE_ASSERT(actor.HasComponent<RigidBodyComponent>(), "Actor doesn't have rigidbody component!");

		RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();
		physx::PxRigidActor* pxActor = RT_CreateRigidActor(actor, rigidbody);
		VX_CORE_ASSERT(pxActor, "Failed to create physics actor!");

		RT_CreateCollider(actor, pxActor);

		// Set Collision Filters
		switch (rigidbody.Type)
		{
			case RigidBodyType::Static:  RT_SetCollisionFilters(pxActor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);  break;
			case RigidBodyType::Dynamic: RT_SetCollisionFilters(pxActor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All); break;
		}

		s_Data->PhysicsScene->addActor(*pxActor);
	}

	void Physics::RT_CreateCharacterControllerInternal(Actor actor)
	{
		VX_CORE_ASSERT(actor.HasComponent<CharacterControllerComponent>(), "Actor doesn't have character controller component!");

		RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();
		physx::PxRigidActor* pxActor = RT_CreateRigidActor(actor, rigidbody);
		VX_CORE_ASSERT(pxActor, "Failed to create physics actor!");

		VX_CORE_ASSERT(!s_Data->ActiveControllers.contains(actor.GetUUID()), "Actors cannot have multiple controllers!");
		physx::PxController* controller = RT_CreateController(actor, pxActor);
		s_Data->ActiveControllers[actor.GetUUID()] = controller;

		CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
		characterController.RuntimeController = (void*)controller;

		pxActor->setName(actor.GetName().c_str());

		RT_SetCollisionFilters(pxActor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);
	}

	void Physics::RT_CreateCollider(Actor actor, physx::PxRigidActor* pxActor)
	{
		if (actor.HasComponent<BoxColliderComponent>())
		{
			AddColliderShape(actor, pxActor, ColliderType::Box);
		}
		else if (actor.HasComponent<SphereColliderComponent>())
		{
			AddColliderShape(actor, pxActor, ColliderType::Sphere);
		}
		else if (actor.HasComponent<CapsuleColliderComponent>())
		{
			AddColliderShape(actor, pxActor, ColliderType::Capsule);
		}
		else if (actor.HasComponent<MeshColliderComponent>())
		{
			// TODO
			//AddColliderShape(actor, pxActor, ColliderType::TriangleMesh);
		}
	}

	void Physics::AddColliderShape(Actor actor, physx::PxRigidActor* pxActor, ColliderType type)
	{
		UUID actorUUID = actor.GetUUID();

		if (!s_Data->ActorColliders.contains(actorUUID))
		{
			s_Data->ActorColliders[actorUUID] = std::vector<SharedReference<ColliderShape>>();
		}

		switch (type)
		{
			case ColliderType::Box:
			{
				BoxColliderComponent& boxCollider = actor.GetComponent<BoxColliderComponent>();
				s_Data->ActorColliders[actorUUID].push_back(SharedReference<BoxColliderShape>::Create(boxCollider, *pxActor, actor));
				break;
			}
			case ColliderType::Sphere:
			{
				SphereColliderComponent& sphereCollider = actor.GetComponent<SphereColliderComponent>();
				s_Data->ActorColliders[actorUUID].push_back(SharedReference<SphereColliderShape>::Create(sphereCollider, *pxActor, actor));
				break;
			}
			case ColliderType::Capsule:
			{
				CapsuleColliderComponent& capsuleCollider = actor.GetComponent<CapsuleColliderComponent>();
				s_Data->ActorColliders[actorUUID].push_back(SharedReference<CapsuleColliderShape>::Create(capsuleCollider, *pxActor, actor));
				break;
			}
			case ColliderType::ConvexMesh:
			{
				MeshColliderComponent& convexMeshCollider = actor.GetComponent<MeshColliderComponent>();
				s_Data->ActorColliders[actorUUID].push_back(SharedReference<ConvexMeshShape>::Create(convexMeshCollider, *pxActor, actor));
				break;
			}
			case ColliderType::TriangleMesh:
			{
				MeshColliderComponent& triangleMeshCollider = actor.GetComponent<MeshColliderComponent>();
				s_Data->ActorColliders[actorUUID].push_back(SharedReference<TriangleMeshShape>::Create(triangleMeshCollider, *pxActor, actor));
				break;
			}
		}
	}

	physx::PxMaterial* Physics::AddControllerColliderShape(Actor actor, physx::PxRigidActor* pxActor, ColliderType type)
	{
		AddColliderShape(actor, pxActor, type);

		return s_Data->ActorColliders[actor.GetUUID()].back()->GetMaterial();
	}

	void Physics::RT_CreateFixedJoint(Actor actor)
	{
		if (!actor)
		{
			VX_CORE_ASSERT(false, "Trying to create fixed joint with invalid actor!");
			return;
		}

		VX_CORE_ASSERT(actor.HasComponent<FixedJointComponent>(), "Actor doesn't have Fixed Joint Component");

		const FixedJointComponent& fixedJointComponent = actor.GetComponent<FixedJointComponent>();
		UUID connectedActorUUID = fixedJointComponent.ConnectedActor;
		Actor connectedActor = s_Data->ContextScene->TryGetActorWithUUID(connectedActorUUID);
		VX_CORE_ASSERT(connectedActor, "Connected Actor was Invalid!");

		if (!connectedActor)
		{
			return;
		}

		physx::PxRigidActor* actor0 = GetActor(actor);
		physx::PxRigidActor* actor1 = GetActor(connectedActor);

		if (!actor0 || !actor1)
		{
			VX_CONSOLE_LOG_ERROR("Unable to create Fixed Joint with entities '{}', '{}'", actor.GetName(), connectedActor.GetName());
			return;
		}

		physx::PxTransform localFrame0 = Utils::GetLocalFrame(actor0);
		physx::PxTransform localFrame1 = Utils::GetLocalFrame(actor1);

		VX_CORE_ASSERT(!s_Data->ActiveFixedJoints.contains(actor.GetUUID()), "Entities can only have one Fixed Joint!");

		physx::PxFixedJoint* fixedJoint = physx::PxFixedJointCreate(*s_Data->PhysXSDK, actor0, localFrame0, actor1, localFrame1);
		s_Data->ActiveFixedJoints[actor.GetUUID()] = fixedJoint;

		ConstrainedJointData* jointData = new ConstrainedJointData();
		jointData->ActorUUID = actor.GetUUID();
		fixedJoint->userData = jointData;

		s_Data->ConstrainedJointData[actor.GetUUID()] = jointData;

		fixedJoint->setBreakForce(fixedJointComponent.BreakForce, fixedJointComponent.BreakTorque);
		fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eCOLLISION_ENABLED, fixedJointComponent.EnableCollision);
		fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eDISABLE_PREPROCESSING, !fixedJointComponent.EnablePreProcessing);
	}

	physx::PxController* Physics::RT_CreateController(Actor actor, physx::PxRigidActor* pxActor)
	{
		const TransformComponent& transform = actor.GetTransform();
		CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();

		physx::PxController* controller = nullptr;

		if (actor.HasComponent<CapsuleColliderComponent>())
		{
			const CapsuleColliderComponent& capsuleCollider = actor.GetComponent<CapsuleColliderComponent>();

			physx::PxMaterial* material = AddControllerColliderShape(actor, pxActor, ColliderType::Capsule);

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
		else if (actor.HasComponent<BoxColliderComponent>())
		{
			const BoxColliderComponent& boxCollider = actor.GetComponent<BoxColliderComponent>();

			physx::PxMaterial* material = AddControllerColliderShape(actor, pxActor, ColliderType::Box);

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

	void Physics::RT_SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask)
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

	void Physics::RT_UpdateDynamicActorProperties(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor)
	{
		dynamicActor->setMass(rigidbody.Mass);
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, rigidbody.IsKinematic);
		dynamicActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, rigidbody.DisableGravity);

		const uint32_t minPositionIterations = s_Data->PositionSolverIterations;
		const uint32_t minVelocityIterations = s_Data->VelocitySolverIterations;
		dynamicActor->setSolverIterationCounts(minPositionIterations, minVelocityIterations);

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
			physx::PxVec3 pxActorVelocity = dynamicActor->getAngularVelocity();
			// If any component of the vector is 0 just use the actors velocity
			Utils::ReplaceInconsistentVectorAxis(angularVelocity, pxActorVelocity);

			dynamicActor->setAngularVelocity(ToPhysXVector(angularVelocity));
		}

		dynamicActor->setMaxAngularVelocity(rigidbody.MaxAngularVelocity);
		dynamicActor->setAngularDamping(rigidbody.AngularDrag);

		uint8_t lockFlags = 0;

		lockFlags |=
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::TranslationX |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::TranslationY |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::TranslationZ |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::RotationX |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::RotationY |
			rigidbody.LockFlags & (uint8_t)ActorLockFlag::RotationZ;

		dynamicActor->setRigidDynamicLockFlags((physx::PxRigidDynamicLockFlags)lockFlags);

		const bool continuousCollisionDetectionEnabled = rigidbody.CollisionDetection == CollisionDetectionType::Continuous;
		const bool continuousSpeculativeCollisionDetectionEnabled = rigidbody.CollisionDetection == CollisionDetectionType::ContinuousSpeculative;
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_CCD, continuousCollisionDetectionEnabled);
		dynamicActor->setRigidBodyFlag(physx::PxRigidBodyFlag::eENABLE_SPECULATIVE_CCD, continuousSpeculativeCollisionDetectionEnabled);

		physx::PxRigidBodyExt::updateMassAndInertia(*dynamicActor, rigidbody.Mass);
	}

	void Physics::InitializeUninitializedActors()
	{
		// Create Rigidbodies, Controllers
		std::vector<UUID> actorsToCreate;

		auto rigidbodyView = s_Data->ContextScene->GetAllActorsWith<TransformComponent, RigidBodyComponent>();

		for (const auto e : rigidbodyView)
		{
			Actor actor{ e, s_Data->ContextScene };

			if (s_Data->ActiveActors.contains(actor.GetUUID()))
				continue;

			actorsToCreate.emplace_back(actor.GetUUID());
		}

		auto controllerView = s_Data->ContextScene->GetAllActorsWith<TransformComponent, RigidBodyComponent, CharacterControllerComponent>();

		for (const auto e : controllerView)
		{
			Actor actor{ e, s_Data->ContextScene };

			if (s_Data->ActiveControllers.contains(actor.GetUUID()))
				continue;

			actorsToCreate.emplace_back(actor.GetUUID());
		}

		std::set<UUID> uniqueActors;
		
		uint32_t size = actorsToCreate.size();
		for (uint32_t i = 0; i < size; i++)
		{
			uniqueActors.insert(actorsToCreate[i]);
		}

		actorsToCreate.assign(uniqueActors.begin(), uniqueActors.end());

		for (UUID uuid : actorsToCreate)
		{
			Actor actor = s_Data->ContextScene->TryGetActorWithUUID(uuid);
			VX_CORE_ASSERT(actor, "Trying to create physics actor on invalid actor!");
			if (!actor)
				continue;

			CreatePhysicsActor(actor);
		}

		// Create Joints
		auto fixedJointView = s_Data->ContextScene->GetAllActorsWith<TransformComponent, RigidBodyComponent, FixedJointComponent>();

		for (const auto e : fixedJointView)
		{
			Actor actor{ e, s_Data->ContextScene };

			if (s_Data->ActiveFixedJoints.contains(actor.GetUUID()))
				continue;

			RT_CreateFixedJoint(actor);
		}
	}

	void Physics::RT_DestroyFixedJointInternal(UUID actorUUID)
	{
		if (!s_Data->ActiveFixedJoints.contains(actorUUID))
		{
			VX_CORE_ASSERT(false, "Trying to destroy fixed joint on invalid actor!");
			return;
		}

		Actor actor = s_Data->ContextScene->TryGetActorWithUUID(actorUUID);
		if (!actor)
		{
			VX_CORE_ASSERT(false, "Trying to destroy fixed joint on invalid actor!");
			return;
		}
		FixedJointComponent& fixedJoint = actor.GetComponent<FixedJointComponent>();
		fixedJoint.ConnectedActor = 0;
		s_Data->ActiveFixedJoints[actorUUID]->release();
		s_Data->ActiveFixedJoints.erase(actorUUID);
	}

	void Physics::RT_DestroyCharacterControllerInternal(UUID actorUUID)
	{
		if (!s_Data->ActiveControllers.contains(actorUUID))
		{
			VX_CORE_ASSERT(false, "Trying to destroy character controller on invalid actor!");
			return;
		}

		Actor actor = s_Data->ContextScene->TryGetActorWithUUID(actorUUID);
		if (!actor)
		{
			VX_CORE_ASSERT(false, "Trying to destroy character controller on invalid actor!");
			return;
		}
		CharacterControllerComponent& characterController = actor.GetComponent<CharacterControllerComponent>();
		physx::PxController* pxController = s_Data->ActiveControllers[actorUUID];
		pxController->release();
		characterController.RuntimeController = nullptr;
		s_Data->ActiveControllers.erase(actorUUID);
	}

	void Physics::RT_DestroyPhysicsActorInternal(UUID actorUUID)
	{
		if (!s_Data->ActiveActors.contains(actorUUID))
		{
			VX_CORE_ASSERT(false, "Trying to destroy physics actor on invalid actor!");
			return;
		}

		Actor actor = s_Data->ContextScene->TryGetActorWithUUID(actorUUID);
		if (!actor)
		{
			VX_CORE_ASSERT(false, "Trying to destroy physics actor on invalid actor!");
			return;
		}
		RigidBodyComponent& rigidbody = actor.GetComponent<RigidBodyComponent>();
		physx::PxRigidActor* pxActor = s_Data->ActiveActors[actorUUID];
		s_Data->PhysicsScene->removeActor(*pxActor);
		pxActor->release();
		rigidbody.RuntimeActor = nullptr;
		s_Data->ActiveActors.erase(actorUUID);
	}

	void Physics::RT_DestroyColliderShapesInternal(UUID actorUUID)
	{
		if (!s_Data->ActorColliders.contains(actorUUID))
		{
			return;
		}

		std::vector<SharedReference<ColliderShape>>& colliderShapes = s_Data->ActorColliders[actorUUID];

		for (auto& colliderShape : colliderShapes)
		{
			physx::PxRigidActor* actor = GetActor(actorUUID);
			if (!actor)
				continue;

			colliderShape->DetachFromActor(actor);
			// TODO shouldn't we be releasing the shape?
			//colliderShape->Release();
		}

		colliderShapes.clear();
		s_Data->ActorColliders.erase(actorUUID);
	}

	void Physics::RT_DestroyPhysicsBodyDataInternal(UUID actorUUID)
	{
		if (!s_Data->PhysicsBodyData.contains(actorUUID))
		{
			return;
		}

		PhysicsBodyData* physicsBodyData = s_Data->PhysicsBodyData[actorUUID];
		if (physicsBodyData == nullptr)
			return;

		delete physicsBodyData;
		s_Data->PhysicsBodyData.erase(actorUUID);
	}

	void Physics::RT_DestroyConstrainedJointDataInternal(UUID actorUUID)
	{
		if (!s_Data->ConstrainedJointData.contains(actorUUID))
		{
			return;
		}

		ConstrainedJointData* jointData = s_Data->ConstrainedJointData[actorUUID];
		if (jointData == nullptr)
			return;

		delete jointData;
		s_Data->ConstrainedJointData.erase(actorUUID);
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
	}

	void Physics::ShutdownPhysicsSceneInternal()
	{
		s_Data->ControllerManager->release();
		s_Data->ControllerManager = nullptr;

		s_Data->PhysicsScene->release();
		s_Data->PhysicsScene = nullptr;

		s_Data->ContextScene = nullptr;

		s_Data->ActiveFixedJoints.clear();
		s_Data->ActiveControllers.clear();
		s_Data->ActiveActors.clear();

		s_Data->ActorColliders.clear();

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

	physx::PxRigidActor* Physics::GetActor(UUID actorUUID)
	{
		VX_CORE_ASSERT(s_Data->ActiveActors.contains(actorUUID), "Actor was not found in active actors map!");

		if (s_Data->ActiveActors.contains(actorUUID))
		{
			return s_Data->ActiveActors[actorUUID];
		}

		return nullptr;
	}

	const std::vector<SharedReference<ColliderShape>>& Physics::GetActorColliders(UUID actorUUID)
	{
		VX_CORE_ASSERT(s_Data->ActorColliders.contains(actorUUID), "Actor doesn't have any colliders!");
		return s_Data->ActorColliders.at(actorUUID);
	}

	physx::PxController* Physics::GetController(UUID actorUUID)
	{
		VX_CORE_ASSERT(s_Data->ActiveControllers.contains(actorUUID), "Actor was not found in active controllers map!");

		if (s_Data->ActiveControllers.contains(actorUUID))
		{
			return s_Data->ActiveControllers[actorUUID];
		}

		return nullptr;
	}

	physx::PxFixedJoint* Physics::GetFixedJoint(UUID actorUUID)
	{
		VX_CORE_ASSERT(s_Data->ActiveFixedJoints.contains(actorUUID), "Actor was not found in active fixed joint map!");

		if (s_Data->ActiveFixedJoints.contains(actorUUID))
		{
			return s_Data->ActiveFixedJoints[actorUUID];
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

	const PhysicsBodyData* Physics::GetPhysicsBodyData(UUID actorUUID)
	{
		VX_CORE_ASSERT(s_Data->PhysicsBodyData.contains(actorUUID), "Actor was not found in physics body data map!");

		if (s_Data->PhysicsBodyData.contains(actorUUID))
		{
			return s_Data->PhysicsBodyData.at(actorUUID);
		}

		return nullptr;
	}

	const ConstrainedJointData* Physics::GetConstrainedJointData(UUID actorUUID)
	{
		VX_CORE_ASSERT(s_Data->ConstrainedJointData.contains(actorUUID), "Actor was not found in constrained joint data map!");

		if (s_Data->ConstrainedJointData.contains(actorUUID))
		{
			return s_Data->ConstrainedJointData.at(actorUUID);
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

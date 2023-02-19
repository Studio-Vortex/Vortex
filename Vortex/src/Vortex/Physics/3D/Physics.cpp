#include "vxpch.h"
#include "Physics.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Physics/3D/PhysXAPIHelpers.h"
#include "Vortex/Physics/3D/PhysicsFilterShader.h"
#include "Vortex/Physics/3D/PhysicsContactListener.h"
#include "Vortex/Scripting/ScriptEngine.h"
#include "Vortex/Utils/PlatformUtils.h"

namespace Vortex {

	namespace Utils {

		static physx::PxBroadPhaseType::Enum VortexBroadphaseTypeToPhysXBroadphaseType(BroadphaseType broadphaseModel)
		{
			switch (broadphaseModel)
			{
				case BroadphaseType::SweepAndPrune:     return physx::PxBroadPhaseType::eSAP;
				case BroadphaseType::MultiBoxPrune:     return physx::PxBroadPhaseType::eMBP;
				case BroadphaseType::AutomaticBoxPrune: return physx::PxBroadPhaseType::eABP;
			}

			VX_CORE_ASSERT(false, "Unknown Broadphase Type!");
			return physx::PxBroadPhaseType::eABP;
		}

		static physx::PxFrictionType::Enum VortexFrictionTypeToPhysXFrictionType(FrictionType frictionModel)
		{
			switch (frictionModel)
			{
				case Vortex::FrictionType::OneDirectional: return physx::PxFrictionType::eONE_DIRECTIONAL;
				case Vortex::FrictionType::Patch:          return physx::PxFrictionType::ePATCH;
				case Vortex::FrictionType::TwoDirectional: return physx::PxFrictionType::eTWO_DIRECTIONAL;
			}

			VX_CORE_ASSERT(false, "Unknown Friction Type!");
			return physx::PxFrictionType::ePATCH;
		}

		static void ReplaceInconsistentVectorAxis(Math::vec3& vector, const physx::PxVec3& replacementVector)
		{
			uint32_t size = vector.length();
			for (uint32_t i = 0; i < size; i++)
			{
				if (vector[i] == 0.0f)
					vector[i] = replacementVector[i];
			}
		}

		static physx::PxTransform GetLocalFrame(physx::PxRigidActor* actor)
		{
			PhysicsBodyData* physicsBodyData = (PhysicsBodyData*)actor->userData;
			Scene* contextScene = physicsBodyData->ContextScene;
			VX_CORE_ASSERT(contextScene, "Context Scene was null pointer!");
			Entity entity = contextScene->TryGetEntityWithUUID(physicsBodyData->EntityUUID);
			VX_CORE_ASSERT(entity, "Invalid Entity UUID!");

			const TransformComponent& worldSpaceTransform = contextScene->GetWorldSpaceTransform(entity);

			Math::quaternion rotation = worldSpaceTransform.GetRotation();
			Math::vec3 globalNormal = rotation * Math::vec3(0.0f, 0.0f, -1.0f);
			Math::vec3 globalAxis = rotation * Math::vec3(0.0f, 1.0f, 0.0f);

			physx::PxVec3 localAnchor = actor->getGlobalPose().transformInv(ToPhysXVector(worldSpaceTransform.Translation));
			physx::PxVec3 localNormal = actor->getGlobalPose().rotateInv(ToPhysXVector(globalNormal));
			physx::PxVec3 localAxis = actor->getGlobalPose().rotateInv(ToPhysXVector(globalAxis));

			physx::PxMat33 rot(localAxis, localNormal, localAxis.cross(localNormal));

			physx::PxTransform localFrame;
			localFrame.p = localAnchor;
			localFrame.q = physx::PxQuat(rot);
			localFrame.q.normalize();

			return localFrame;
		}

	}

	struct PhysicsEngineInternalData
	{
		physx::PxDefaultAllocator DefaultAllocator;
		physx::PxDefaultErrorCallback ErrorCallback;
		physx::PxFoundation* Foundation = nullptr;
		physx::PxPhysics* PhysXSDK = nullptr;
		physx::PxDefaultCpuDispatcher* Dispatcher = nullptr;
		physx::PxCooking* CookingFactory = nullptr;
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
	};

	static PhysicsEngineInternalData* s_Data = nullptr;

	void Physics::OnSimulationStart(Scene* contextScene)
	{
		InitPhysicsSceneInternal();

		s_ActiveActors.clear();
		s_ActiveControllers.clear();
		s_ActiveFixedJoints.clear();

		s_Data->ContextScene = contextScene;

		TraverseSceneForUninitializedActors();
	}

	void Physics::OnSimulationUpdate(TimeStep delta)
	{
		s_Data->PhysicsScene->simulate(delta);
		s_Data->PhysicsScene->fetchResults(true);
		s_Data->PhysicsScene->setGravity(ToPhysXVector(s_PhysicsSceneGravity));

		TraverseSceneForUninitializedActors();

		for (const auto& [entityUUID, actor] : s_ActiveActors)
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

		for (const auto& [entityUUID, characterController] : s_ActiveControllers)
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

		for (const auto& [entityUUID, fixedJoint] : s_ActiveFixedJoints)
		{
			physx::PxVec3 linear(0.0f), angular(0.0f);
			physx::PxFixedJoint* fixedJoint = s_ActiveFixedJoints[entityUUID];
			fixedJoint->getConstraint()->getForce(linear, angular);

			Math::vec3 linearForce = FromPhysXVector(linear);
			Math::vec3 angularForce = FromPhysXVector(angular);
			s_LastReportedJointForces[fixedJoint] = std::make_pair(linearForce, angularForce);
		}

#ifndef VX_DIST
		s_Data->PhysicsScene->getSimulationStatistics(s_Data->SimulationStats);
#endif
	}

	void Physics::OnSimulationStop(Scene* contextScene)
	{
		std::vector<UUID> actorsToDestroy;

		for (const auto& [entityUUID, fixedJoint] : s_ActiveFixedJoints)
		{
			actorsToDestroy.push_back(entityUUID);
		}

		for (const auto& [entityUUID, characterController] : s_ActiveControllers)
		{
			actorsToDestroy.push_back(entityUUID);
		}

		for (const auto& [entityUUID, actor] : s_ActiveActors)
		{
			actorsToDestroy.push_back(entityUUID);
		}

		for (const auto& entityUUID : actorsToDestroy)
		{
			Entity entity = s_Data->ContextScene->TryGetEntityWithUUID(entityUUID);
			DestroyPhysicsActor(entity);
		}

		ShutdownPhysicsSceneInternal();
	}

	void Physics::ReCreateActor(Entity entity)
	{
		DestroyPhysicsActor(entity);
		CreatePhysicsActor(entity);
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
			if (physx::PxRigidDynamic* actor = buffer[i]->is<physx::PxRigidDynamic>())
			{
				bool gravityDisabled = actor->getActorFlags() & physx::PxActorFlag::eDISABLE_GRAVITY;
				bool isAwake = !actor->isSleeping();

				if (gravityDisabled || isAwake)
					continue;

				actor->wakeUp();
			}
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

	physx::PxRigidActor* Physics::GetActor(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_ActiveActors.contains(entityUUID), "Entity was not found in active actors map!");

		if (s_ActiveActors.contains(entityUUID))
		{
			return s_ActiveActors[entityUUID];
		}
		
		return nullptr;
	}

	physx::PxController* Physics::GetController(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_ActiveControllers.contains(entityUUID), "Entity was not found in active controllers map!");

		if (s_ActiveControllers.contains(entityUUID))
		{
			return s_ActiveControllers[entityUUID];
		}

		return nullptr;
	}

	physx::PxFixedJoint* Physics::GetFixedJoint(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_ActiveFixedJoints.contains(entityUUID), "Entity was not found in active fixed joint map!");
		
		if (s_ActiveFixedJoints.contains(entityUUID))
		{
			return s_ActiveFixedJoints[entityUUID];
		}

		return nullptr;
	}

	const std::pair<Math::vec3, Math::vec3>& Physics::GetLastReportedFixedJointForces(physx::PxFixedJoint* fixedJoint)
	{
		VX_CORE_ASSERT(s_LastReportedJointForces.contains(fixedJoint), "Fixed Joint was not found in last reported forces map!");

		if (s_LastReportedJointForces.contains(fixedJoint))
		{
			return s_LastReportedJointForces[fixedJoint];
		}

		return std::make_pair(Math::vec3(), Math::vec3());
	}

	const PhysicsBodyData* Physics::GetPhysicsBodyData(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_PhysicsBodyData.contains(entityUUID), "Entity was not found in physics body data map!");

		if (s_PhysicsBodyData.contains(entityUUID))
		{
			return s_PhysicsBodyData.at(entityUUID);
		}

		return nullptr;
	}

	const ConstrainedJointData* Physics::GetConstrainedJointData(UUID entityUUID)
	{
		VX_CORE_ASSERT(s_ConstrainedJointData.contains(entityUUID), "Entity was not found in constrained joint data map!");

		if (s_ConstrainedJointData.contains(entityUUID))
		{
			return s_ConstrainedJointData.at(entityUUID);
		}

		return nullptr;
	}

	void Physics::BreakJoint(UUID entityUUID)
	{
		if (s_ActiveFixedJoints.contains(entityUUID))
		{
			physx::PxFixedJoint* fixedJoint = s_ActiveFixedJoints[entityUUID];
			fixedJoint->setConstraintFlag(physx::PxConstraintFlag::eBROKEN, true);
		}
	}

	void Physics::Init()
	{
		InitPhysicsSDKInternal();
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

		physx::PxCookingParams cookingParameters = physx::PxCookingParams(s_Data->TolerancesScale);
		cookingParameters.midphaseDesc = physx::PxMeshMidPhase::eBVH34;

		s_Data->CookingFactory = PxCreateCooking(PX_PHYSICS_VERSION, *s_Data->Foundation, cookingParameters);
		VX_CORE_ASSERT(s_Data->CookingFactory, "Failed to Initialize PhysX Cooking!");
	}

	void Physics::InitPhysicsSceneInternal()
	{
		physx::PxSceneDesc sceneDescription = physx::PxSceneDesc(s_Data->TolerancesScale);
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_CCD | physx::PxSceneFlag::eENABLE_PCM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		sceneDescription.flags |= physx::PxSceneFlag::eENABLE_ACTIVE_ACTORS;

		sceneDescription.gravity = ToPhysXVector(s_PhysicsSceneGravity);

		SharedRef<Project> activeProject = Project::GetActive();
		const ProjectProperties& projectProps = activeProject->GetProperties();

		sceneDescription.broadPhaseType = Utils::VortexBroadphaseTypeToPhysXBroadphaseType(projectProps.PhysicsProps.BroadphaseModel);
		sceneDescription.frictionType = Utils::VortexFrictionTypeToPhysXFrictionType(projectProps.PhysicsProps.FrictionModel);

		sceneDescription.cpuDispatcher = s_Data->Dispatcher;
		sceneDescription.filterShader = PhysicsFilterShader::FilterShader;
		sceneDescription.simulationEventCallback = &s_Data->ContactListener;

		s_Data->PhysicsScene = s_Data->PhysXSDK->createScene(sceneDescription);
		s_Data->ControllerManager = PxCreateControllerManager(*s_Data->PhysicsScene);
	}

	void Physics::Shutdown()
	{
		ShutdownPhysicsSDKInternal();
	}

	void Physics::ShutdownPhysicsSDKInternal()
	{
		s_Data->CookingFactory->release();
		s_Data->Dispatcher->release();
		PxCloseExtensions();
		s_Data->PhysXSDK->release();
		s_Data->Foundation->release();

		delete s_Data;
	}

	void Physics::ShutdownPhysicsSceneInternal()
	{
		s_Data->ControllerManager->release();
		s_Data->ControllerManager = nullptr;
		s_Data->PhysicsScene->release();
		s_Data->PhysicsScene = nullptr;

		s_Data->ContextScene = nullptr;
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
			Physics::UpdateDynamicActorProperties(rigidbody, dynamicActor);
			actor = dynamicActor;
		}

		VX_CORE_ASSERT(actor != nullptr, "Failed to create Physics Actor!");
		rigidbody.RuntimeActor = (void*)actor;

		UUID entityUUID = entity.GetUUID();
		PhysicsBodyData* physicsBodyData = new PhysicsBodyData();
		physicsBodyData->EntityUUID = entityUUID;
		physicsBodyData->ContextScene = entity.GetContextScene();
		actor->userData = physicsBodyData;

		s_PhysicsBodyData[entityUUID] = physicsBodyData;

		if (entity.HasComponent<CharacterControllerComponent>())
		{
			VX_CORE_ASSERT(!s_ActiveControllers.contains(entity.GetUUID()), "Entities cannot have multiple controllers!");

			physx::PxController* controller = CreateController(entity);
			s_ActiveControllers[entityUUID] = controller;
			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.RuntimeController = (void*)controller;

			Physics::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);

			return;
		}

		s_ActiveActors[entityUUID] = actor;
		CreateCollider(entity);

		// Set Filters
		if (rigidbody.Type == RigidBodyType::Static)
			Physics::SetCollisionFilters(actor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);
		else if (rigidbody.Type == RigidBodyType::Dynamic)
			Physics::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);

		s_Data->PhysicsScene->addActor(*actor);
	}

	void Physics::DestroyPhysicsActor(Entity entity)
	{
		if (!entity.HasComponent<RigidBodyComponent>())
			return;

		const UUID entityUUID = entity.GetUUID();

		if (s_ActiveActors.contains(entityUUID))
		{
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
			physx::PxRigidActor* actor = s_ActiveActors[entityUUID];
			s_Data->PhysicsScene->removeActor(*actor);
			actor->release();
			rigidbody.RuntimeActor = nullptr;
			s_ActiveActors.erase(entity);
		}

		if (s_ActiveControllers.contains(entityUUID))
		{
			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();
			characterController.RuntimeController = nullptr;
			s_ActiveControllers[entityUUID]->release();
			s_ActiveControllers.erase(entity);
		}

		if (s_ActiveFixedJoints.contains(entityUUID))
		{
			FixedJointComponent& fixedJoint = entity.GetComponent<FixedJointComponent>();
			fixedJoint.ConnectedEntity = 0;
			s_ActiveFixedJoints[entityUUID]->release();
			s_ActiveFixedJoints.erase(entity);
		}

		if (s_PhysicsBodyData.contains(entityUUID))
		{
			PhysicsBodyData* physicsBodyData = s_PhysicsBodyData[entityUUID];
			delete physicsBodyData;
			s_PhysicsBodyData.erase(entity);
		}

		if (s_ConstrainedJointData.contains(entityUUID))
		{
			ConstrainedJointData* jointData = s_ConstrainedJointData[entityUUID];
			delete jointData;
			s_ConstrainedJointData.erase(entity);
		}
	}

	void Physics::CreateCollider(Entity entity)
	{
		const auto& transform = entity.GetTransform();

		physx::PxRigidActor* actor = (physx::PxRigidActor*)entity.GetComponent<RigidBodyComponent>().RuntimeActor;

		if (entity.HasComponent<BoxColliderComponent>())
		{
			const auto& boxCollider = entity.GetComponent<BoxColliderComponent>();
			Math::vec3 scale = transform.Scale;

			physx::PxBoxGeometry boxGeometry = physx::PxBoxGeometry(boxCollider.HalfSize.x * scale.x, boxCollider.HalfSize.y * scale.y, boxCollider.HalfSize.z * scale.z);
			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = CreatePhysicsMaterial(physicsMaterial);
			}
			else
			{
				// Create a default material
				material = CreatePhysicsMaterial(PhysicsMaterialComponent());
			}

			physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, boxGeometry, *material);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !boxCollider.IsTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, boxCollider.IsTrigger);
			shape->setLocalPose(ToPhysXTransform(Math::Translate(boxCollider.Offset)));
		}

		if (entity.HasComponent<SphereColliderComponent>())
		{
			const auto& sphereCollider = entity.GetComponent<SphereColliderComponent>();

			float largestComponent = Math::Max(transform.Scale.x, Math::Max(transform.Scale.y, transform.Scale.z));
			physx::PxSphereGeometry sphereGeometry = physx::PxSphereGeometry(sphereCollider.Radius * largestComponent);
			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = CreatePhysicsMaterial(physicsMaterial);
			}
			else
			{
				// Create a default material
				material = CreatePhysicsMaterial(PhysicsMaterialComponent());
			}

			physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, sphereGeometry, *material);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !sphereCollider.IsTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, sphereCollider.IsTrigger);
			shape->setLocalPose(ToPhysXTransform(Math::Translate(sphereCollider.Offset)));
		}

		if (entity.HasComponent<CapsuleColliderComponent>())
		{
			const auto& capsuleCollider = entity.GetComponent<CapsuleColliderComponent>();

			physx::PxCapsuleGeometry capsuleGeometry = physx::PxCapsuleGeometry(capsuleCollider.Radius, capsuleCollider.Height / 2.0F);
			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = CreatePhysicsMaterial(physicsMaterial);
			}
			else
			{
				// Create a default material
				material = CreatePhysicsMaterial(PhysicsMaterialComponent());
			}

			physx::PxShape* shape = physx::PxRigidActorExt::createExclusiveShape(*actor, capsuleGeometry, *material);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, !capsuleCollider.IsTrigger);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, capsuleCollider.IsTrigger);
			shape->setLocalPose(ToPhysXTransform(Math::Translate(capsuleCollider.Offset)));

			// Make sure that the capsule is facing up (+Y)
			shape->setLocalPose(physx::PxTransform(physx::PxQuat(physx::PxHalfPi, physx::PxVec3(0, 0, 1))));
		}

		if (entity.HasComponent<MeshColliderComponent>())
		{
			const auto& meshCollider = entity.GetComponent<MeshColliderComponent>();
		}
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

		VX_CORE_ASSERT(!s_ActiveFixedJoints.contains(entity.GetUUID()), "Entities can only have one Fixed Joint!");

		physx::PxFixedJoint* fixedJoint = physx::PxFixedJointCreate(*s_Data->PhysXSDK, actor0, localFrame0, actor1, localFrame1);
		s_ActiveFixedJoints[entity.GetUUID()] = fixedJoint;

		ConstrainedJointData* jointData = new ConstrainedJointData();
		jointData->EntityUUID = entity.GetUUID();
		jointData->IsBroken = false;
		fixedJoint->userData = jointData;

		s_ConstrainedJointData[entity.GetUUID()] = jointData;

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

			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = CreatePhysicsMaterial(physicsMaterial);
			}
			else
			{
				// Create a default material
				material = CreatePhysicsMaterial(PhysicsMaterialComponent());
			}

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

			physx::PxMaterial* material = nullptr;

			if (entity.HasComponent<PhysicsMaterialComponent>())
			{
				const auto& physicsMaterial = entity.GetComponent<PhysicsMaterialComponent>();
				material = CreatePhysicsMaterial(physicsMaterial);
			}
			else
			{
				// Create a default material
				material = CreatePhysicsMaterial(PhysicsMaterialComponent());
			}

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

	physx::PxMaterial* Physics::CreatePhysicsMaterial(const PhysicsMaterialComponent& component)
	{
		physx::PxMaterial* material = s_Data->PhysXSDK->createMaterial(component.StaticFriction, component.DynamicFriction, component.Bounciness);
		material->setFrictionCombineMode((physx::PxCombineMode::Enum)component.FrictionCombineMode);
		material->setRestitutionCombineMode((physx::PxCombineMode::Enum)component.RestitutionCombineMode);
		return material;
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

	void Physics::UpdateDynamicActorProperties(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor)
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

			if (rigidbody.RuntimeActor || s_ActiveActors.contains(entity.GetUUID()))
				continue;

			CreatePhysicsActor(entity);
		}

		auto characterControllerView = s_Data->ContextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent, CharacterControllerComponent>();

		for (const auto e : characterControllerView)
		{
			Entity entity{ e, s_Data->ContextScene };
			const CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();

			if (characterController.RuntimeController || s_ActiveControllers.contains(entity.GetUUID()))
				continue;

			CreatePhysicsActor(entity);
		}

		auto fixedJointView = s_Data->ContextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent, FixedJointComponent>();

		for (const auto e : fixedJointView)
		{
			Entity entity{ e, s_Data->ContextScene };

			if (s_ActiveFixedJoints.contains(entity.GetUUID()))
				continue;

			CreateFixedJoint(entity);
		}
	}

}

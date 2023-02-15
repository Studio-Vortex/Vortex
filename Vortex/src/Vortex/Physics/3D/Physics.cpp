#include "vxpch.h"
#include "Physics.h"

#include "Vortex/Project/Project.h"
#include "Vortex/Renderer/Mesh.h"
#include "Vortex/Renderer/StaticMesh.h"
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

	}

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

		if (!s_Data->PhysicsScene)
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

	void Physics::OnSimulationStart(Scene* contextScene)
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

		s_Data->PhysicsScene = s_Data->PhysicsFactory->createScene(sceneDescription);
		s_Data->ControllerManager = PxCreateControllerManager(*s_Data->PhysicsScene);

		auto view = contextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, contextScene };
			CreatePhysicsActor(entity);
		}

		s_Data->ContextScene = contextScene;
	}

	void Physics::OnSimulationUpdate(TimeStep delta, Scene* contextScene)
	{
		s_Data->PhysicsScene->simulate(s_FixedTimeStep);
		s_Data->PhysicsScene->fetchResults(true);
		s_Data->PhysicsScene->setGravity(ToPhysXVector(s_PhysicsSceneGravity));

		auto view = contextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, contextScene };

			auto& transform = entity.GetTransform();

			if (entity.HasComponent<RigidBodyComponent>())
			{
				auto& rigidbody = entity.GetComponent<RigidBodyComponent>();

				if (!rigidbody.RuntimeActor)
				{
					CreatePhysicsActor(entity);
				}

				physx::PxRigidActor* actor = static_cast<physx::PxRigidActor*>(rigidbody.RuntimeActor);

				if (rigidbody.Type == RigidBodyType::Dynamic)
				{
					physx::PxRigidDynamic* dynamicActor = static_cast<physx::PxRigidDynamic*>(actor);

					entity.SetTransform(FromPhysXTransform(dynamicActor->getGlobalPose()) * Math::Scale(transform.Scale));

					if (actor->is<physx::PxRigidDynamic>())
					{
						UpdateDynamicActorFlags(rigidbody, dynamicActor);
					}
				}
				else if (rigidbody.Type == RigidBodyType::Static)
				{
					// Synchronize with entity Transform
					actor->setGlobalPose(ToPhysXTransform(transform));
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

#ifndef VX_DIST
		s_Data->PhysicsScene->getSimulationStatistics(s_Data->SimulationStats);
#endif
	}

	void Physics::OnSimulationStop(Scene* contextScene)
	{
		auto view = contextScene->GetAllEntitiesWith<TransformComponent, RigidBodyComponent>();

		for (const auto& e : view)
		{
			Entity entity{ e, contextScene };
			DestroyPhysicsActor(entity);
		}

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
			actor = Physics::GetPhysicsFactory()->createRigidStatic(ToPhysXTransform(transform));
		}
		else if (rigidbody.Type == RigidBodyType::Dynamic)
		{
			physx::PxRigidDynamic* dynamicActor = Physics::GetPhysicsFactory()->createRigidDynamic(ToPhysXTransform(transform));
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
			CharacterControllerComponent& characterController = entity.GetComponent<CharacterControllerComponent>();

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

				float radiusScale = Math::Max(transform.Scale.x, transform.Scale.y);

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

				characterController.RuntimeController = (void*)s_Data->ControllerManager->createController(desc);
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

				characterController.RuntimeController = (void*)s_Data->ControllerManager->createController(desc);
			}
		}
		else
		{
			CreateCollider(entity);
		}

		// Set Filters
		if (rigidbody.Type == RigidBodyType::Static)
			Physics::SetCollisionFilters(actor, (uint32_t)FilterGroup::Static, (uint32_t)FilterGroup::All);
		else if (rigidbody.Type == RigidBodyType::Dynamic)
			Physics::SetCollisionFilters(actor, (uint32_t)FilterGroup::Dynamic, (uint32_t)FilterGroup::All);

		s_Data->PhysicsScene->addActor(*actor);
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

	physx::PxMaterial* Physics::CreatePhysicsMaterial(const PhysicsMaterialComponent& component)
	{
		physx::PxMaterial* material = Physics::GetPhysicsFactory()->createMaterial(component.StaticFriction, component.DynamicFriction, component.Bounciness);
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

	void Physics::DestroyPhysicsActor(Entity entity)
	{
		physx::PxActor* actor = nullptr;

		if (entity.HasComponent<RigidBodyComponent>())
		{
			auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
			physx::PxActor* actor = static_cast<physx::PxActor*>(rigidbody.RuntimeActor);

			if (actor == nullptr)
			{
				VX_CONSOLE_LOG_WARN("Trying to delete Physics Actor that doesn't exist!");
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

	physx::PxSimulationStatistics* Physics::GetSimulationStatistics()
	{
		return &s_Data->SimulationStats;
	}

}

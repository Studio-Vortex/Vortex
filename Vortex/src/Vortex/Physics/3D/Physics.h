#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/TimeStep.h"

#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Physics/3D/PhysicsData.h"
#include "Vortex/Physics/3D/PhysicsShapes.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Entity.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include <unordered_map>

namespace physx {

	class PxScene;
	class PxPhysics;
	class PxMaterial;
	class PxController;
	class PxFixedJoint;
	class PxRigidActor;
	class PxRigidDynamic;
	class PxControllerManager;
	class PxSimulationStatistics;
	class PxTolerancesScale;
	class PxFoundation;
	class PxPhysics;

}

namespace Vortex {

	class VORTEX_API Physics
	{
	public:
		static void Init();
		static void Shutdown();

		static void OnSimulationStart(Scene* contextScene);
		static void OnSimulationUpdate(TimeStep delta);
		static void OnSimulationStop(Scene* contextScene);

		static void CreatePhysicsActor(Entity entity);
		static void ReCreateActor(Entity entity);
		static void DestroyPhysicsActor(Entity entity);

		static void WakeUpActor(Entity entity);
		static void WakeUpActors();

		static bool Raycast(const Math::vec3& origin, const Math::vec3& direction, float maxDistance, RaycastHit* outHitInfo);

		static bool IsConstraintBroken(UUID entityUUID);
		static void BreakJoint(UUID entityUUID);

		static void OnCharacterControllerUpdateRuntime(UUID entityUUID, const Math::vec3& displacement);

	public:
		static const std::unordered_map<UUID, physx::PxRigidActor*>& GetActors();
		static const std::unordered_map<UUID, physx::PxController*>& GetControllers();
		static const std::unordered_map<UUID, physx::PxFixedJoint*> GetFixedJoints();

		static physx::PxRigidActor* GetActor(UUID entityUUID);
		static const std::vector<SharedReference<ColliderShape>>& GetEntityColliders(UUID entityUUID);
		static physx::PxController* GetController(UUID entityUUID);
		static physx::PxFixedJoint* GetFixedJoint(UUID entityUUID);

		static const std::pair<Math::vec3, Math::vec3>& GetLastReportedFixedJointForces(physx::PxFixedJoint* fixedJoint);

		static const PhysicsBodyData* GetPhysicsBodyData(UUID entityUUID);
		static const ConstrainedJointData* GetConstrainedJointData(UUID entityUUID);

		static Scene* GetContextScene();

	public:
		static physx::PxScene* GetPhysicsScene();

#ifndef VX_DIST
		static physx::PxSimulationStatistics* GetSimulationStatistics();
#endif

		static uint32_t GetPhysicsScenePositionIterations();
		static void SetPhysicsScenePositionIterations(uint32_t positionIterations);

		static uint32_t GetPhysicsSceneVelocityIterations();
		static void SetPhysicsSceneVelocityIterations(uint32_t veloctiyIterations);

		static Math::vec3 GetPhysicsSceneGravity();
		static void SetPhysicsSceneGravity(const Math::vec3& gravity);

		static physx::PxPhysics* GetPhysicsSDK();
		static physx::PxTolerancesScale* GetTolerancesScale();
		static physx::PxFoundation* GetFoundation();

	private:
		static void InitPhysicsSDKInternal();
		static void InitPhysicsSceneInternal();

		static void ShutdownPhysicsSDKInternal();
		static void ShutdownPhysicsSceneInternal();

	private:
		static void SimulationStep();

		static void UpdateActors();
		static void UpdateControllers();
		static void UpdateFixedJoints();

		static void RegisterPhysicsActor(Entity entity, physx::PxRigidActor* actor);
		static physx::PxRigidActor* CreateRuntimeActor(Entity entity, RigidBodyComponent& rigidbody);
		static void CreatePhysicsActorInternal(Entity entity);
		static void CreateCharacterControllerInternal(Entity entity);

		static void CreateCollider(Entity entity, physx::PxRigidActor* actor);
		static void AddColliderShape(Entity entity, physx::PxRigidActor* actor, ColliderType type);
		static physx::PxMaterial* AddControllerColliderShape(Entity entity, physx::PxRigidActor* actor, ColliderType type);
		static void CreateFixedJoint(Entity entity);
		static physx::PxController* CreateController(Entity entity, physx::PxRigidActor* actor);

		static void SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);
		static void UpdateDynamicActorProperties(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor);
		static void InitializeUninitializedActors();

		static void DestroyFixedJointInternal(UUID entityUUID);
		static void DestroyCharacterControllerInternal(UUID entityUUID);
		static void DestroyPhysicsActorInternal(UUID entityUUID);
		static void DestroyColliderShapesInternal(UUID entityUUID);
		static void DestroyPhysicsBodyDataInternal(UUID entityUUID);
		static void DestroyConstrainedJointDataInternal(UUID entityUUID);
	};

}

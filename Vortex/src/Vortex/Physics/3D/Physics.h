#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/TimeStep.h"

#include "Vortex/Physics/3D/PhysXTypes.h"
#include "Vortex/Physics/3D/PhysicsData.h"
#include "Vortex/Physics/3D/PhysicsShapes.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"
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

		static void CreatePhysicsActor(Actor actor);
		static void ReCreateActor(Actor actor);
		static void DestroyPhysicsActor(Actor actor);

		static void WakeUpActor(Actor actor);
		static void WakeUpActors();

		static bool Raycast(const Math::vec3& origin, const Math::vec3& direction, float maxDistance, RaycastHit* outHitInfo);

		static bool IsConstraintBroken(UUID actorUUID);
		static void BreakJoint(UUID actorUUID);

		static void OnCharacterControllerUpdateRuntime(UUID actorUUID, const Math::vec3& displacement);

	public:
		static const std::unordered_map<UUID, physx::PxRigidActor*>& GetActors();
		static const std::unordered_map<UUID, physx::PxController*>& GetControllers();
		static const std::unordered_map<UUID, physx::PxFixedJoint*> GetFixedJoints();

		static physx::PxRigidActor* GetActor(UUID actorUUID);
		static const std::vector<SharedReference<ColliderShape>>& GetActorColliders(UUID actorUUID);
		static physx::PxController* GetController(UUID actorUUID);
		static physx::PxFixedJoint* GetFixedJoint(UUID actorUUID);

		static const std::pair<Math::vec3, Math::vec3>& GetLastReportedFixedJointForces(physx::PxFixedJoint* fixedJoint);

		static const PhysicsBodyData* GetPhysicsBodyData(UUID actorUUID);
		static const ConstrainedJointData* GetConstrainedJointData(UUID actorUUID);

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

		static void RegisterPhysicsActor(Actor actor, physx::PxRigidActor* pxActor);
		static physx::PxRigidActor* CreateRuntimeActor(Actor actor, RigidBodyComponent& rigidbody);
		static void CreatePhysicsActorInternal(Actor actor);
		static void CreateCharacterControllerInternal(Actor actor);

		static void CreateCollider(Actor actor, physx::PxRigidActor* pxActor);
		static void AddColliderShape(Actor actor, physx::PxRigidActor* pxActor, ColliderType type);
		static physx::PxMaterial* AddControllerColliderShape(Actor actor, physx::PxRigidActor* pxActor, ColliderType type);
		static void CreateFixedJoint(Actor actor);
		static physx::PxController* CreateController(Actor actor, physx::PxRigidActor* pxActor);

		static void SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);
		static void UpdateDynamicActorProperties(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor);
		static void InitializeUninitializedActors();

		static void DestroyFixedJointInternal(UUID actorUUID);
		static void DestroyCharacterControllerInternal(UUID actorUUID);
		static void DestroyPhysicsActorInternal(UUID actorUUID);
		static void DestroyColliderShapesInternal(UUID actorUUID);
		static void DestroyPhysicsBodyDataInternal(UUID actorUUID);
		static void DestroyConstrainedJointDataInternal(UUID actorUUID);
	};

}

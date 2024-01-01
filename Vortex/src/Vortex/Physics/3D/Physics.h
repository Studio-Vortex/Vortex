#pragma once

#include "Vortex/Core/Base.h"

#include "Vortex/Core/UUID.h"
#include "Vortex/Core/TimeStep.h"

#include "Vortex/Physics/3D/PhysicsTypes.h"
#include "Vortex/Physics/3D/PhysicsShapes.h"

#include "Vortex/Scene/Scene.h"
#include "Vortex/Scene/Actor.h"
#include "Vortex/Scene/Components.h"

#include "Vortex/ReferenceCounting/SharedRef.h"

#include <unordered_map>

namespace physx {

	class PxMaterial;
	class PxController;
	class PxFixedJoint;
	class PxRigidActor;
	class PxRigidDynamic;
	class PxControllerManager;
	class PxSimulationStatistics;

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

		static bool IsPhysicsActor(UUID actorUUID);

		static void CreatePhysicsActor(Actor actor);
		static void ReCreateActor(Actor actor);
		static void DestroyPhysicsActor(Actor actor);

		static void WakeUpActor(Actor actor);

		static bool IsConstraintBroken(UUID actorUUID);
		static void BreakJoint(UUID actorUUID);

		static void RT_DisplaceCharacterController(TimeStep delta, UUID actorUUID, const Math::vec3& displacement);

		static const std::unordered_map<UUID, physx::PxRigidActor*>& GetPhysicsActors();
		static const std::unordered_map<UUID, physx::PxController*>& GetControllers();
		static const std::unordered_map<UUID, physx::PxFixedJoint*> GetFixedJoints();

		static physx::PxRigidActor* GetPhysicsActor(UUID actorUUID);
		static const std::vector<SharedReference<ColliderShape>>& GetActorColliders(UUID actorUUID);
		static physx::PxController* GetController(UUID actorUUID);
		static physx::PxFixedJoint* GetFixedJoint(UUID actorUUID);

		static const std::pair<Math::vec3, Math::vec3>& GetLastReportedFixedJointForces(physx::PxFixedJoint* fixedJoint);

		static const PhysicsBodyData* GetPhysicsBodyData(UUID actorUUID);
		static const ConstrainedJointData* GetConstrainedJointData(UUID actorUUID);

		static Scene* GetContextScene();

		static void* GetDispatcher();

#ifndef VX_DIST
		static physx::PxSimulationStatistics* GetSimulationStatistics();
#endif

		static void* GetPhysicsSDK();
		static void* GetTolerancesScale();
		static void* GetFoundation();

	private:
		static void InitPhysicsSDKInternal();
		static void InitPhysicsSceneInternal();

		static void ShutdownPhysicsSDKInternal();
		static void ShutdownPhysicsSceneInternal();

		static void RT_SimulationStep();

		static void RT_UpdateActors();
		static void RT_UpdateControllers();
		static void RT_UpdateJoints();

		static void RT_RegisterPhysicsActor(Actor actor, physx::PxRigidActor* pxActor);
		static physx::PxRigidActor* RT_CreateRigidActor(Actor actor, RigidBodyComponent& rigidbody);
		static void RT_CreatePhysicsActorInternal(Actor actor);
		static void RT_CreateCharacterControllerInternal(Actor actor);

		static void RT_CreateCollider(Actor actor, physx::PxRigidActor* pxActor);
		static void AddColliderShape(Actor actor, physx::PxRigidActor* pxActor, ColliderType type);
		static std::vector<SharedReference<ColliderShape>>& GetActorColliderShapesInternal(UUID actorUUID);
		static physx::PxMaterial* AddControllerColliderShape(Actor actor, physx::PxRigidActor* pxActor, ColliderType type);
		static void RT_CreateFixedJoint(Actor actor);
		static physx::PxController* RT_CreateController(Actor actor, physx::PxRigidActor* pxActor);

		static void RT_SetCollisionFilters(physx::PxRigidActor* actor, uint32_t filterGroup, uint32_t filterMask);
		static void RT_UpdateDynamicActorProperties(const RigidBodyComponent& rigidbody, physx::PxRigidDynamic* dynamicActor);
		static void InitializeUninitializedActors();

		static void RT_DestroyFixedJointInternal(UUID actorUUID);
		static void RT_DestroyCharacterControllerInternal(UUID actorUUID);
		static void RT_DestroyPhysicsActorInternal(UUID actorUUID);
		static void RT_DestroyColliderShapesInternal(UUID actorUUID);
		static void RT_DestroyPhysicsBodyDataInternal(UUID actorUUID);
		static void RT_DestroyConstrainedJointDataInternal(UUID actorUUID);
	};

}
